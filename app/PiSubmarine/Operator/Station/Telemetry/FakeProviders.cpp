#include "PiSubmarine/Operator/Station/Telemetry/FakeProviders.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    namespace
    {
        class FakeLampProvider final : public ::PiSubmarine::Lamp::Telemetry::Api::IProvider
        {
        public:
            [[nodiscard]] Error::Api::Result<::PiSubmarine::Lamp::Telemetry::Api::Status> GetStatus() const override
            {
                ++m_Tick;
                return ::PiSubmarine::Lamp::Telemetry::Api::Status{
                    .IsActive = (m_Tick / 10) % 2 == 0,
                    .HasOvertemperatureWarning = (m_Tick / 25) % 2 == 1};
            }

        private:
            mutable int m_Tick = 0;
        };

        class FakeMotorProvider final : public ::PiSubmarine::Motor::Telemetry::Api::IProvider
        {
        public:
            explicit FakeMotorProvider(const int phaseOffset)
                : m_Tick(phaseOffset)
            {
            }

            [[nodiscard]] Error::Api::Result<::PiSubmarine::Motor::Telemetry::Api::State> GetState() const override
            {
                ++m_Tick;
                const auto operational = (m_Tick / 20) % 3 == 0
                    ? ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Operational
                    : ((m_Tick / 20) % 3 == 1
                        ? ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Degraded
                        : ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Faulted);

                return ::PiSubmarine::Motor::Telemetry::Api::State{
                    .Operational = operational,
                    .ActiveFaults = operational == ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Faulted
                        ? ::PiSubmarine::Motor::Telemetry::Api::Faults::Overcurrent
                        : static_cast<::PiSubmarine::Motor::Telemetry::Api::Faults>(0),
                    .ActiveWarnings = operational == ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Degraded
                        ? ::PiSubmarine::Motor::Telemetry::Api::Warnings::Temperature
                        : static_cast<::PiSubmarine::Motor::Telemetry::Api::Warnings>(0)};
            }

        private:
            mutable int m_Tick = 0;
        };

        class FakeBatteryProvider final : public ::PiSubmarine::Battery::Telemetry::Api::IProvider
        {
        public:
            [[nodiscard]] Error::Api::Result<::PiSubmarine::Battery::Telemetry::Api::State> GetState() const override
            {
                ++m_Tick;
                const auto cycle = static_cast<double>(m_Tick % 100) / 100.0;
                return ::PiSubmarine::Battery::Telemetry::Api::State{
                    .PackVoltage = Volts{15.8 - cycle},
                    .ChargerVoltage = Volts{16.4},
                    .PackCurrent = Amperes{1.5 - cycle},
                    .ChargerCurrent = Amperes{0.4},
                    .ChargerTemperature = Celsius{28.0},
                    .PackTemperature = Celsius{30.0 + (cycle * 6.0)},
                    .MonitorTemperature = Celsius{31.0},
                    .RemainingCapacity = AmpereHours{3.4},
                    .StateOfCharge = NormalizedFraction(1.0 - cycle)};
            }

        private:
            mutable int m_Tick = 0;
        };
    }

    FakeProviders CreateFakeProviders(const std::size_t motorCount)
    {
        FakeProviders providers;
        providers.Lamp = std::make_unique<FakeLampProvider>();
        providers.Battery = std::make_unique<FakeBatteryProvider>();
        providers.Motors.reserve(motorCount);

        for (std::size_t index = 0; index < motorCount; ++index)
        {
            providers.Motors.push_back(std::make_unique<FakeMotorProvider>(static_cast<int>(index) * 7));
        }

        return providers;
    }
}
