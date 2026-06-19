#pragma once

#include <memory>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
	// TODO FakeController and normal Controller use inconsistent ownership models. Controller.cpp accepts sub-controllers as dependencies, FakeController creates them instead. Consider removing FakeController and using Controller.cpp only. To simulate Telemetry inject fake domain-specific Telemetry providers to sub-controllers and fake lease issuer to Controller.cpp
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
		PiSubmarine::Logging::Api::IFactory& loggerFactory,
		QObject* parent = nullptr);
}
