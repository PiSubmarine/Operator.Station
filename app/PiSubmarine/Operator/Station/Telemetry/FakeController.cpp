#include "PiSubmarine/Operator/Station/Telemetry/FakeController.h"

#include <chrono>

#include "PiSubmarine/Battery/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Lamp/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Motor/Telemetry/Api/IProvider.h"
#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"

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

    FakeParts CreateFakeController(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ISubscriptionService& subscriptionService,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
    {
        static FakeLampProvider lampProvider;
        static FakeMotorProvider motorProvider;
        static FakeBatteryProvider batteryProvider;

        FakeParts parts;
        parts.Lamp = std::make_unique<LampController>(lampProvider, parent);
        parts.Motor = std::make_unique<MotorController>(motorProvider, parent);
        parts.Battery = std::make_unique<BatteryController>(batteryProvider, parent);
        parts.Coordinator = std::make_unique<Controller>(
            leaseIssuer,
            subscriptionService,
            *parts.Lamp,
            *parts.Motor,
            *parts.Battery,
            loggerFactory,
            parent);
        return parts;
    }
}
