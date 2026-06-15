#pragma once

#include <chrono>
#include <memory>
#include <optional>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Lease/Api/LeaseGrant.h"
#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Video/Config.h"
#include "PiSubmarine/Operator/Station/Video/IPipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"
#include "PiSubmarine/Operator/Station/Video/Status.h"
#include "PiSubmarine/Time/ITickable.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace spdlog
{
    class logger;
}

namespace PiSubmarine::Operator::Station::Video
{
    class IPipeline;

    class VideoController final : public Time::ITickable
    {
    public:
        VideoController(
            Config config,
            Logging::Api::IFactory& loggerFactory,
            Lease::Api::ILeaseIssuer& leaseIssuer,
            ::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
            std::shared_ptr<IPipelineBuilder> pipelineBuilder);
        ~VideoController() override;

        [[nodiscard]] Error::Api::Result<void> Start();
        [[nodiscard]] Error::Api::Result<void> Stop();
        [[nodiscard]] Error::Api::Result<Status> GetStatus() const;
        [[nodiscard]] Error::Api::Result<void> SetPipelineTailFactory(IVideoPipelineTailFactory& tailFactory);
        [[nodiscard]] Error::Api::Result<void> SetReceiveEndpoint(const ReceiveEndpoint& receiveEndpoint);
        [[nodiscard]] Error::Api::Result<void> SetSubscriptionEndpoint(
            const ::PiSubmarine::Video::Subscription::Api::Endpoint& subscriptionEndpoint);
        void ClearPipelineTailFactory();

        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    private:
        [[nodiscard]] Error::Api::Result<void> AcquireLease(const std::chrono::nanoseconds& uptime);
        [[nodiscard]] Error::Api::Result<void> RenewLease(const std::chrono::nanoseconds& uptime);
        [[nodiscard]] Error::Api::Result<void> EnsureSubscribed();
        [[nodiscard]] Error::Api::Result<void> RebuildPipeline();
        void ResetLeaseState() noexcept;
        void ScheduleRetry(const std::chrono::nanoseconds& uptime) noexcept;

        Config m_Config;
        std::shared_ptr<spdlog::logger> m_Logger;
        Lease::Api::ILeaseIssuer& m_LeaseIssuer;
        ::PiSubmarine::Video::Subscription::Api::IService& m_SubscriptionService;
        std::shared_ptr<IPipelineBuilder> m_PipelineBuilder;
        std::unique_ptr<IPipeline> m_Pipeline;
        IVideoPipelineTailFactory* m_TailFactory = nullptr;
        std::optional<Lease::Api::LeaseGrant> m_LeaseGrant;
        std::chrono::nanoseconds m_NextLeaseRenewal{0};
        std::chrono::nanoseconds m_NextRetryTime{0};
        bool m_IsStarted = false;
        bool m_IsSubscribed = false;
        bool m_IsDirty = true;
    };
}
