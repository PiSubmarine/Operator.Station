#include "PiSubmarine/Operator/Station/Composition/FakeVideo.h"

#include "PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h"

namespace PiSubmarine::Operator::Station::Composition
{
    FakeVideo::FakeVideo(
        ::PiSubmarine::Logging::Api::IFactory& loggerFactory,
        ::PiSubmarine::Operator::Station::Video::View::QmlVideoSinkTailFactory& tailFactory)
        : m_PipelineBuilder(::PiSubmarine::Operator::Station::Video::CreateFakePipelineBuilder(loggerFactory, tailFactory))
    {
    }

    std::shared_ptr<::PiSubmarine::Operator::Station::Video::IPipelineBuilder> FakeVideo::GetPipelineBuilder()
    {
        return m_PipelineBuilder;
    }

    ::PiSubmarine::Video::Subscription::Api::IService& FakeVideo::GetSubscriptionService()
    {
        return m_SubscriptionService;
    }

    ::PiSubmarine::Error::Api::Result<void> FakeVideo::LocalSubscriptionService::Subscribe(
        const ::PiSubmarine::Video::Subscription::Api::SubscribeRequest& request)
    {
        m_LastRequest = request;
        return {};
    }

    ::PiSubmarine::Error::Api::Result<void> FakeVideo::LocalSubscriptionService::Unsubscribe(
        const ::PiSubmarine::Video::Subscription::Api::UnsubscribeRequest& request)
    {
        m_LastLeaseId = request.LeaseId;
        return {};
    }
}
