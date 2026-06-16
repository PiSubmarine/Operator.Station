#pragma once

#include <memory>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    struct FakeParts
    {
        std::unique_ptr<LampController> Lamp;
        std::unique_ptr<MotorController> Motor;
        std::unique_ptr<BatteryController> Battery;
        std::unique_ptr<Controller> Coordinator;
    };

    [[nodiscard]] FakeParts CreateFakeController(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ISubscriptionService& subscriptionService,
        Logging::Api::IFactory& loggerFactory,
        QObject* parent = nullptr);
}
