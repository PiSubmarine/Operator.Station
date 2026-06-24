#pragma once

#include <memory>
#include <vector>

#include "PiSubmarine/Ballast/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Battery/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Depth/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Lamp/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Motor/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Proximity/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Time/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Video/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    struct FakeProviders
    {
        std::unique_ptr<::PiSubmarine::Ballast::Telemetry::Api::IProvider> Ballast;
        std::unique_ptr<::PiSubmarine::Lamp::Telemetry::Api::IProvider> Lamp;
        std::unique_ptr<::PiSubmarine::Motor::Telemetry::Api::IProvider> BallastMotor;
        std::vector<std::unique_ptr<::PiSubmarine::Motor::Telemetry::Api::IProvider>> Motors;
        std::unique_ptr<::PiSubmarine::Battery::Telemetry::Api::IProvider> Battery;
        std::unique_ptr<::PiSubmarine::Depth::Telemetry::Api::IProvider> Depth;
        std::unique_ptr<::PiSubmarine::Proximity::Telemetry::Api::IProvider> Proximity;
        std::unique_ptr<::PiSubmarine::Time::Telemetry::Api::IProvider> Time;
        std::unique_ptr<::PiSubmarine::Video::Telemetry::Api::IProvider> Video;
    };

    [[nodiscard]] FakeProviders CreateFakeProviders(std::size_t motorCount);
}
