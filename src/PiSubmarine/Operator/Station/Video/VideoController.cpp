#include "PiSubmarine/Operator/Station/Video/VideoController.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/Operator/Station/Video/IPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    namespace
    {
        [[nodiscard]] Error::Api::Error MakeCommunicationError()
        {
            return Error::Api::MakeError(Error::Api::ErrorCondition::CommunicationError);
        }
    }

    VideoController::VideoController(
        Config config,
        Logging::Api::IFactory& loggerFactory,
        Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
        std::shared_ptr<IPipelineBuilder> pipelineBuilder)
        : m_Config(std::move(config))
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Video"))
        , m_LeaseIssuer(leaseIssuer)
        , m_SubscriptionService(subscriptionService)
        , m_PipelineBuilder(std::move(pipelineBuilder))
    {
        if (!m_Logger)
        {
            throw std::invalid_argument("Operator.Station.Video requires a logger");
        }

        if (!m_PipelineBuilder)
        {
            throw std::invalid_argument("Operator.Station.Video requires a pipeline builder");
        }
    }

    VideoController::~VideoController() = default;

    Error::Api::Result<void> VideoController::Start()
    {
        m_IsStarted = true;
        m_IsDirty = true;
        m_NextRetryTime = std::chrono::nanoseconds::zero();
        SPDLOG_LOGGER_INFO(m_Logger, "Video controller started");
        return {};
    }

    Error::Api::Result<void> VideoController::Stop()
    {
        m_IsStarted = false;
        m_IsDirty = true;
        m_NextRetryTime = std::chrono::nanoseconds::zero();

        std::optional<Error::Api::Error> firstError;

        if (m_Pipeline)
        {
            const auto stopResult = m_Pipeline->Stop();
            if (!stopResult.has_value() && !firstError.has_value())
            {
                firstError = stopResult.error();
            }

            m_Pipeline.reset();
        }

        if (m_IsSubscribed && m_LeaseGrant.has_value())
        {
            const auto unsubscribeResult =
                m_SubscriptionService.Unsubscribe({.LeaseId = m_LeaseGrant->Lease.Id});
            if (!unsubscribeResult.has_value() && !firstError.has_value())
            {
                firstError = unsubscribeResult.error();
            }
        }

        if (m_LeaseGrant.has_value())
        {
            const auto releaseResult = m_LeaseIssuer.ReleaseLease(m_LeaseGrant->Lease.Id);
            if (!releaseResult.has_value() && !firstError.has_value())
            {
                firstError = releaseResult.error();
            }
        }

        ResetLeaseState();

        if (firstError.has_value())
        {
            return std::unexpected(*firstError);
        }

        SPDLOG_LOGGER_INFO(m_Logger, "Video controller stopped");
        return {};
    }

    Error::Api::Result<Status> VideoController::GetStatus() const
    {
        return Status{
            .IsStarted = m_IsStarted,
            .HasTailFactory = m_TailFactory != nullptr,
            .HasLease = m_LeaseGrant.has_value(),
            .IsSubscribed = m_IsSubscribed,
            .IsPipelineRunning = m_Pipeline != nullptr && m_Pipeline->IsRunning(),
            .LeaseId = m_LeaseGrant.has_value() ? std::optional(m_LeaseGrant->Lease.Id) : std::nullopt};
    }

    Error::Api::Result<void> VideoController::SetPipelineTailFactory(IVideoPipelineTailFactory& tailFactory)
    {
        m_TailFactory = &tailFactory;
        m_IsDirty = true;
        return {};
    }

    Error::Api::Result<void> VideoController::SetReceiveEndpoint(const ReceiveEndpoint& receiveEndpoint)
    {
        if (m_Config.ReceiveEndpoint == receiveEndpoint)
        {
            return {};
        }

        m_Config.ReceiveEndpoint = receiveEndpoint;
        m_IsDirty = true;
        return {};
    }

    Error::Api::Result<void> VideoController::SetSubscriptionEndpoint(
        const ::PiSubmarine::Video::Subscription::Api::Endpoint& subscriptionEndpoint)
    {
        if (m_Config.SubscriptionEndpoint == subscriptionEndpoint)
        {
            return {};
        }

        m_Config.SubscriptionEndpoint = subscriptionEndpoint;
        if (m_IsSubscribed && m_LeaseGrant.has_value())
        {
            static_cast<void>(m_SubscriptionService.Unsubscribe({.LeaseId = m_LeaseGrant->Lease.Id}));
        }

        m_IsSubscribed = false;
        m_IsDirty = true;
        return {};
    }

    void VideoController::ClearPipelineTailFactory()
    {
        m_TailFactory = nullptr;
        m_IsDirty = true;

        if (m_Pipeline)
        {
            static_cast<void>(m_Pipeline->Stop());
            m_Pipeline.reset();
        }
    }

    void VideoController::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        static_cast<void>(deltaTime);

        if (m_Pipeline)
        {
            m_Pipeline->PollBus();
        }

        if (!m_IsStarted)
        {
            return;
        }

        if (uptime < m_NextRetryTime)
        {
            return;
        }

        if (!m_LeaseGrant.has_value())
        {
            const auto acquireResult = AcquireLease(uptime);
            if (!acquireResult.has_value())
            {
                SPDLOG_LOGGER_WARN(m_Logger, "Failed to acquire video lease");
                ScheduleRetry(uptime);
                return;
            }
        }

        if (uptime >= m_NextLeaseRenewal)
        {
            const auto renewResult = RenewLease(uptime);
            if (!renewResult.has_value())
            {
                SPDLOG_LOGGER_WARN(m_Logger, "Failed to renew video lease");
                ScheduleRetry(uptime);
                return;
            }
        }

        if (!m_IsSubscribed)
        {
            const auto subscribeResult = EnsureSubscribed();
            if (!subscribeResult.has_value())
            {
                SPDLOG_LOGGER_WARN(m_Logger, "Failed to subscribe video stream");
                ScheduleRetry(uptime);
                return;
            }
        }

        const auto shouldRebuild =
            m_TailFactory != nullptr &&
            (m_IsDirty || !m_Pipeline || !m_Pipeline->IsRunning());
        if (shouldRebuild)
        {
            const auto rebuildResult = RebuildPipeline();
            if (!rebuildResult.has_value())
            {
                SPDLOG_LOGGER_WARN(m_Logger, "Failed to rebuild video pipeline");
                ScheduleRetry(uptime);
            }
        }
        else if (m_TailFactory == nullptr && m_Pipeline)
        {
            static_cast<void>(m_Pipeline->Stop());
            m_Pipeline.reset();
        }
    }

    Error::Api::Result<void> VideoController::AcquireLease(const std::chrono::nanoseconds& uptime)
    {
        const auto leaseResult = m_LeaseIssuer.AcquireLease({.Resource = m_Config.ResourceId});
        if (!leaseResult.has_value())
        {
            return std::unexpected(leaseResult.error());
        }

        m_LeaseGrant = *leaseResult;
        m_IsSubscribed = false;
        m_IsDirty = true;
        m_NextLeaseRenewal = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_LeaseGrant->Lease.Duration / 2);
        return {};
    }

    Error::Api::Result<void> VideoController::RenewLease(const std::chrono::nanoseconds& uptime)
    {
        if (!m_LeaseGrant.has_value())
        {
            return std::unexpected(MakeCommunicationError());
        }

        const auto renewResult = m_LeaseIssuer.RenewLease(m_LeaseGrant->Lease.Id);
        if (!renewResult.has_value())
        {
            if (m_Pipeline)
            {
                static_cast<void>(m_Pipeline->Stop());
                m_Pipeline.reset();
            }

            ResetLeaseState();
            return std::unexpected(renewResult.error());
        }

        m_LeaseGrant->Lease = *renewResult;
        m_NextLeaseRenewal = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_LeaseGrant->Lease.Duration / 2);
        return {};
    }

    Error::Api::Result<void> VideoController::EnsureSubscribed()
    {
        if (!m_LeaseGrant.has_value())
        {
            return std::unexpected(MakeCommunicationError());
        }

        const auto subscribeResult = m_SubscriptionService.Subscribe({
            .LeaseId = m_LeaseGrant->Lease.Id,
            .ClientEndpoint = m_Config.SubscriptionEndpoint});
        if (!subscribeResult.has_value())
        {
            return std::unexpected(subscribeResult.error());
        }

        m_IsSubscribed = true;
        return {};
    }

    Error::Api::Result<void> VideoController::RebuildPipeline()
    {
        if (m_TailFactory == nullptr)
        {
            return {};
        }

        if (m_Pipeline)
        {
            const auto stopResult = m_Pipeline->Stop();
            if (!stopResult.has_value())
            {
                return stopResult;
            }
        }

        try
        {
            SPDLOG_LOGGER_INFO(
                m_Logger,
                "Rebuilding video pipeline for {}:{}",
                m_Config.ReceiveEndpoint.BindAddress,
                m_Config.ReceiveEndpoint.Port);
            m_Pipeline = m_PipelineBuilder->Build(m_Config.ReceiveEndpoint, *m_TailFactory);
        }
        catch (...)
        {
            return std::unexpected(Error::Api::MakeError(Error::Api::ErrorCondition::DeviceError));
        }

        m_IsDirty = false;
        SPDLOG_LOGGER_INFO(m_Logger, "Video pipeline is running");
        return {};
    }

    void VideoController::ResetLeaseState() noexcept
    {
        m_IsSubscribed = false;
        m_LeaseGrant.reset();
        m_NextLeaseRenewal = std::chrono::nanoseconds::zero();
    }

    void VideoController::ScheduleRetry(const std::chrono::nanoseconds& uptime) noexcept
    {
        m_NextRetryTime = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_Config.RetryDelay);
    }
}
