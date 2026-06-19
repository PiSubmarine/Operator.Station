#include "PiSubmarine/Operator/Station/Video/FakeController.h"

#include "PiSubmarine/Operator/Station/Video/Controller.h"
#include "PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"

namespace PiSubmarine::Operator::Station::Video
{
    std::unique_ptr<Controller> CreateFakeController(
        Config config,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
        IVideoPipelineTailFactory& tailFactory,
        QObject* parent)
    {
        return std::make_unique<Controller>(
            std::move(config),
            loggerFactory,
            leaseIssuer,
            subscriptionService,
            CreateFakePipelineBuilder(loggerFactory),
            tailFactory,
            parent);
    }
}
