#pragma once

#include <memory>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Lease/Api/Identifiers.h"
#include "PiSubmarine/Operator/Station/Composition/IVideo.h"
#include "PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"
#include "PiSubmarine/Video/Subscription/Api/SubscribeRequest.h"
#include "PiSubmarine/Video/Subscription/Api/UnsubscribeRequest.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    class QmlVideoSinkTailFactory;
}

namespace PiSubmarine::Operator::Station::Composition
{
    class FakeVideo final : public IVideo
    {
    public:
        FakeVideo(
            ::PiSubmarine::Logging::Api::IFactory& loggerFactory,
            ::PiSubmarine::Operator::Station::Video::View::QmlVideoSinkTailFactory& tailFactory);

        [[nodiscard]] std::shared_ptr<::PiSubmarine::Operator::Station::Video::IPipelineBuilder> GetPipelineBuilder() override;
        [[nodiscard]] ::PiSubmarine::Video::Subscription::Api::IService& GetSubscriptionService() override;

    private:
        class LocalSubscriptionService final : public ::PiSubmarine::Video::Subscription::Api::IService
        {
        public:
            [[nodiscard]] ::PiSubmarine::Error::Api::Result<void> Subscribe(
                const ::PiSubmarine::Video::Subscription::Api::SubscribeRequest& request) override;
            [[nodiscard]] ::PiSubmarine::Error::Api::Result<void> Unsubscribe(
                const ::PiSubmarine::Video::Subscription::Api::UnsubscribeRequest& request) override;

        private:
            ::PiSubmarine::Video::Subscription::Api::SubscribeRequest m_LastRequest{};
            ::PiSubmarine::Lease::Api::LeaseId m_LastLeaseId{};
        };

        std::shared_ptr<::PiSubmarine::Operator::Station::Video::IPipelineBuilder> m_PipelineBuilder;
        LocalSubscriptionService m_SubscriptionService;
    };
}
