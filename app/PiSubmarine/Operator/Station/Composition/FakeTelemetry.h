#pragma once

#include <cstddef>

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
        [[nodiscard]] ::PiSubmarine::Motor::Telemetry::Api::IProvider& GetBallastMotor() override;
        [[nodiscard]] std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>>
        GetMotors() override;
        [[nodiscard]] ::PiSubmarine::Proximity::Telemetry::Api::IProvider& GetProximity() override;
        [[nodiscard]] ::PiSubmarine::Time::Telemetry::Api::IProvider& GetTime() override;
        [[nodiscard]] ::PiSubmarine::Video::Telemetry::Api::IProvider& GetVideo() override;
        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    private:
        Telemetry::FakeProviders m_Providers;
        std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>> m_Motors;
        bool m_HasLeaseState = false;
    };
}
