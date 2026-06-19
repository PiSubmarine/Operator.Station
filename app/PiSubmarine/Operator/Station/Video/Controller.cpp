#include "PiSubmarine/Operator/Station/Video/Controller.h"

#include <stdexcept>
#include <QMetaObject>
#include <QThread>

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

    Controller::Controller(
        Config config,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
        std::shared_ptr<IPipelineBuilder> pipelineBuilder,
        QObject* parent)
        : QObject(parent)
        , m_Config(std::move(config))
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Video.Controller"))
        , m_LeaseIssuer(leaseIssuer)
        , m_SubscriptionService(subscriptionService)
        , m_PipelineBuilder(std::move(pipelineBuilder))
    {
        if (!m_Logger || !m_PipelineBuilder)
        {
            throw std::invalid_argument("Video controller requires logger and pipeline builder");
        }

        m_Timer.setParent(this);
        m_Timer.setInterval(16);
        connect(&m_Timer, &QTimer::timeout, this, &Controller::TickNow);
    }

    Controller::~Controller()
    {
        Stop();
    }

    void Controller::Start()
    {
        if (m_IsStarted)
        {
            return;
        }

        m_IsStarted = true;
        m_IsDirty = true;
        m_NextRetryTime = std::chrono::nanoseconds::zero();
        m_StartTime = std::chrono::steady_clock::now();
        m_LastTickTime = m_StartTime;

        m_Timer.start();
    }

    void Controller::Stop()
    {
        if (thread() != nullptr && QThread::currentThread() != thread())
        {
            if (thread()->isRunning())
            {
                QMetaObject::invokeMethod(this, &Controller::Stop, Qt::BlockingQueuedConnection);
            }
            return;
        }

        m_Timer.stop();

        if (!m_IsStarted && !m_Pipeline && !m_LeaseGrant.has_value())
        {
            return;
        }

        m_IsStarted = false;
        m_IsDirty = true;
        m_NextRetryTime = std::chrono::nanoseconds::zero();

        if (m_Pipeline)
        {
            static_cast<void>(m_Pipeline->Stop());
            m_Pipeline.reset();
        }

        if (m_IsSubscribed && m_LeaseGrant.has_value())
        {
            static_cast<void>(m_SubscriptionService.Unsubscribe({.LeaseId = m_LeaseGrant->Lease.Id}));
        }

        if (m_LeaseGrant.has_value())
        {
            const auto releaseResult = m_LeaseIssuer.ReleaseLease(m_LeaseGrant->Lease.Id);
            if (!releaseResult.has_value() && !IsNotReadyError(releaseResult.error()))
            {
                SPDLOG_LOGGER_WARN(m_Logger, "Failed to release video lease");
            }
        }

        ResetLeaseState();
    }

    void Controller::SetReceiveEndpoint(const QString& bindAddress, const quint16 port)
    {
        const ReceiveEndpoint nextEndpoint{
            .BindAddress = bindAddress.toStdString(),
            .Port = port};

        if (m_Config.ReceiveEndpoint == nextEndpoint)
        {
            return;
        }

        m_Config.ReceiveEndpoint = nextEndpoint;
        m_IsDirty = true;
    }

    void Controller::SetSubscriptionEndpoint(const QString& host, const quint16 port)
    {
        const ::PiSubmarine::Video::Subscription::Api::Endpoint nextEndpoint{
            .Host = host.toStdString(),
            .Port = port};

        if (m_Config.SubscriptionEndpoint == nextEndpoint)
        {
            return;
        }

        m_Config.SubscriptionEndpoint = nextEndpoint;
        m_IsSubscribed = false;
        m_IsDirty = true;
    }

    Status Controller::GetStatus() const
    {
        return {
            .IsStarted = m_IsStarted,
            .HasLease = m_LeaseGrant.has_value(),
            .IsSubscribed = m_IsSubscribed,
            .IsPipelineRunning = m_Pipeline != nullptr && m_Pipeline->IsRunning(),
            .LeaseId = m_LeaseGrant.has_value() ? std::optional(m_LeaseGrant->Lease.Id) : std::nullopt};
    }

    void Controller::TickNow()
    {
        const auto now = std::chrono::steady_clock::now();
        const auto uptime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_StartTime);
        const auto delta = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_LastTickTime);
        m_LastTickTime = now;
        Tick(uptime, delta);
    }

    void Controller::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        static_cast<void>(deltaTime);

        if (m_Pipeline)
        {
            m_Pipeline->PollBus();
        }

        if (!m_IsStarted || uptime < m_NextRetryTime)
        {
            return;
        }

        if (!m_LeaseGrant.has_value())
        {
            const auto acquireResult = AcquireLease(uptime);
            if (!acquireResult.has_value())
            {
                if (!IsNotReadyError(acquireResult.error()))
                {
                    ScheduleRetry(uptime);
                }
                return;
            }
        }

        if (uptime >= m_NextLeaseRenewal)
        {
            const auto renewResult = RenewLease(uptime);
            if (!renewResult.has_value())
            {
                if (!IsNotReadyError(renewResult.error()))
                {
                    ScheduleRetry(uptime);
                }
                return;
            }
        }

        if (!m_IsSubscribed)
        {
            const auto subscribeResult = EnsureSubscribed();
            if (!subscribeResult.has_value())
            {
                ScheduleRetry(uptime);
                return;
            }
        }

        if (m_IsDirty || !m_Pipeline || !m_Pipeline->IsRunning())
        {
            const auto rebuildResult = RebuildPipeline();
            if (!rebuildResult.has_value())
            {
                ScheduleRetry(uptime);
            }
        }
    }

    Error::Api::ErrorCondition Controller::GetNotReadyCondition()
    {
        return Error::Api::ErrorCondition::NotReady;
    }

    bool Controller::IsNotReadyError(const Error::Api::Error& error)
    {
        return error.Condition == GetNotReadyCondition();
    }

    Error::Api::Result<void> Controller::AcquireLease(const std::chrono::nanoseconds& uptime)
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

    Error::Api::Result<void> Controller::RenewLease(const std::chrono::nanoseconds& uptime)
    {
        if (!m_LeaseGrant.has_value())
        {
            return std::unexpected(MakeCommunicationError());
        }

        const auto renewResult = m_LeaseIssuer.RenewLease(m_LeaseGrant->Lease.Id);
        if (!renewResult.has_value())
        {
            if (IsNotReadyError(renewResult.error()))
            {
                return {};
            }

            ResetLeaseState();
            return std::unexpected(renewResult.error());
        }

        m_LeaseGrant->Lease = *renewResult;
        m_NextLeaseRenewal = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_LeaseGrant->Lease.Duration / 2);
        return {};
    }

    Error::Api::Result<void> Controller::EnsureSubscribed()
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

    Error::Api::Result<void> Controller::RebuildPipeline()
    {
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
            m_Pipeline = m_PipelineBuilder->Build(m_Config.ReceiveEndpoint);
        }
        catch (const std::exception& exception)
        {
            SPDLOG_LOGGER_ERROR(m_Logger, "Failed to build pipeline: {}", exception.what());
            return std::unexpected(Error::Api::MakeError(Error::Api::ErrorCondition::DeviceError));
        }

        const auto playResult = m_Pipeline->Play();
        if (!playResult.has_value())
        {
            return playResult;
        }

        m_IsDirty = false;
        return {};
    }

    void Controller::ResetLeaseState() noexcept
    {
        m_IsSubscribed = false;
        m_LeaseGrant.reset();
        m_NextLeaseRenewal = std::chrono::nanoseconds::zero();
    }

    void Controller::ScheduleRetry(const std::chrono::nanoseconds& uptime) noexcept
    {
        m_NextRetryTime = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_Config.RetryDelay);
    }
}
