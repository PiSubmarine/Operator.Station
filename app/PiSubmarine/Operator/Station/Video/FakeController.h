#pragma once

#include <memory>

#include <QObject>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Video/Config.h"

namespace PiSubmarine::Video::Subscription::Api
{
    class IService;
}

namespace PiSubmarine::Operator::Station::Video
{
    class Controller;
    class IVideoPipelineTailFactory;

    [[nodiscard]] std::unique_ptr<Controller> CreateFakeController(
        Config config,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Video::Subscription::Api::IService& subscriptionService,
        IVideoPipelineTailFactory& tailFactory,
        QObject* parent = nullptr);
}
