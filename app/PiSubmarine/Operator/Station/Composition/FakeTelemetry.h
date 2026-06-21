#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "PiSubmarine/Operator/Station/Composition/ITelemetry.h"
#include "PiSubmarine/Operator/Station/Telemetry/FakeProviders.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class FakeTelemetry final : public ITelemetry
    {
    public:
        explicit FakeTelemetry(std::size_t motorCount);

        [[nodiscard]] ::PiSubmarine::Ballast::Telemetry::Api::IProvider& GetBallast() override;
        [[nodiscard]] ::PiSubmarine::Battery::Telemetry::Api::IProvider& GetBattery() override;
        [[nodiscard]] ::PiSubmarine::Depth::Telemetry::Api::IProvider& GetDepth() override;
        [[nodiscard]] ::PiSubmarine::Lamp::Telemetry::Api::IProvider& GetLamp() override;
        [[nodiscard]] std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>>
        GetMotors() override;
        [[nodiscard]] ::PiSubmarine::Proximity::Telemetry::Api::IProvider& GetProximity() override;
        [[nodiscard]] ::PiSubmarine::Video::Telemetry::Api::IProvider& GetVideo() override;
        [[nodiscard]] std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> GetTickables() override;

    private:
        Telemetry::FakeProviders m_Providers;
        std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>> m_Motors;
    };
}
