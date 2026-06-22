#pragma once

#include <QObject>

#include "PiSubmarine/Ballast/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Battery/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Depth/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Lamp/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Motor/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Operator/Station/Composition/LeaseState.h"
#include "PiSubmarine/Proximity/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Time/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Time/ITickable.h"
#include "PiSubmarine/Video/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Composition
{
	class ITelemetry : public QObject, public ::PiSubmarine::Time::ITickable
	{
        Q_OBJECT

	public:
        explicit ITelemetry(QObject* parent = nullptr)
            : QObject(parent)
        {
        }

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

    signals:
        void LeaseStateChanged(const OptionalLeaseId& leaseId);
	};
}
