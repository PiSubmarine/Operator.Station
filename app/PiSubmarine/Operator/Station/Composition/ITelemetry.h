#pragma once

#include <functional>
#include <vector>

#include "PiSubmarine/Ballast/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Battery/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Depth/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Lamp/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Motor/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Proximity/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Time/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Time/ITickable.h"
#include "PiSubmarine/Video/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Composition
{
	class ITelemetry
	{
	public:
		virtual ~ITelemetry() = default;

		[[nodiscard]] virtual ::PiSubmarine::Ballast::Telemetry::Api::IProvider& GetBallast() = 0;
		[[nodiscard]] virtual ::PiSubmarine::Battery::Telemetry::Api::IProvider& GetBattery() = 0;
		[[nodiscard]] virtual ::PiSubmarine::Depth::Telemetry::Api::IProvider& GetDepth() = 0;
		[[nodiscard]] virtual ::PiSubmarine::Lamp::Telemetry::Api::IProvider& GetLamp() = 0;
		[[nodiscard]] virtual std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>>
		GetMotors() = 0;
		[[nodiscard]] virtual ::PiSubmarine::Proximity::Telemetry::Api::IProvider& GetProximity() = 0;
		[[nodiscard]] virtual ::PiSubmarine::Time::Telemetry::Api::IProvider& GetTime() = 0;
		[[nodiscard]] virtual ::PiSubmarine::Video::Telemetry::Api::IProvider& GetVideo() = 0;
		[[nodiscard]] virtual bool HasLease() const = 0;
		[[nodiscard]] virtual std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> GetTickables() = 0;
	};
}
