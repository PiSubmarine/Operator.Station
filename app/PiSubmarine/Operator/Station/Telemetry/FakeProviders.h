#pragma once

#include <memory>
#include <vector>

#include "PiSubmarine/Battery/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Lamp/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Motor/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    struct FakeProviders
    {
        std::unique_ptr<::PiSubmarine::Lamp::Telemetry::Api::IProvider> Lamp;
        std::vector<std::unique_ptr<::PiSubmarine::Motor::Telemetry::Api::IProvider>> Motors;
        std::unique_ptr<::PiSubmarine::Battery::Telemetry::Api::IProvider> Battery;
    };

    [[nodiscard]] FakeProviders CreateFakeProviders(std::size_t motorCount);
}
