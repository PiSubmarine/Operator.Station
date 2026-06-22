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
                    .Intensity = (m_Tick / 10) % 2 == 0 ? NormalizedFraction{0.8} : NormalizedFraction{0},
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
                        : static_cast<::PiSubmarine::Motor::Telemetry::Api::Warnings>(0),
                    .Direction = operational == ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Faulted
                        ? ::PiSubmarine::Motor::Telemetry::Api::DriveDirection::Idle
                        : ::PiSubmarine::Motor::Telemetry::Api::DriveDirection::Forward,
                    .DriveEffort = operational == ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Operational
                        ? NormalizedFraction{0.8}
                        : operational == ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Degraded
                            ? NormalizedFraction{0.45}
                            : NormalizedFraction{0}};
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
                    .StateOfCharge = NormalizedFraction(1.0 - cycle),
                    .TimeToFull = std::chrono::milliseconds{(20 + (m_Tick % 40)) * 60000},
                    .TimeToEmpty = std::chrono::milliseconds{(90 - (m_Tick % 50)) * 60000}};
            }

        private:
            mutable int m_Tick = 0;
        };

        class FakeBallastProvider final : public ::PiSubmarine::Ballast::Telemetry::Api::IProvider
        {
        public:
            [[nodiscard]] Error::Api::Result<::PiSubmarine::Ballast::Telemetry::Api::State> GetState() const override
            {
                ++m_Tick;
                const auto cycle = static_cast<double>(m_Tick % 100) / 100.0;
                return ::PiSubmarine::Ballast::Telemetry::Api::State{
                    .Position = NormalizedFraction(cycle)};
            }

        private:
            mutable int m_Tick = 0;
        };

        class FakeDepthProvider final : public ::PiSubmarine::Depth::Telemetry::Api::IProvider
        {
        public:
            [[nodiscard]] Error::Api::Result<::PiSubmarine::Depth::Telemetry::Api::State> GetState() const override
            {
                ++m_Tick;
                const auto cycle = static_cast<double>(m_Tick % 80) / 20.0;
                return ::PiSubmarine::Depth::Telemetry::Api::State{
                    .Depth = Meters{1.0 + cycle}};
            }

        private:
            mutable int m_Tick = 0;
        };

        class FakeProximityProvider final : public ::PiSubmarine::Proximity::Telemetry::Api::IProvider
        {
        public:
            [[nodiscard]] Error::Api::Result<::PiSubmarine::Proximity::Telemetry::Api::State> GetState() const override
            {
                ++m_Tick;
                const auto cycle = static_cast<double>(m_Tick % 60) / 30.0;
                return ::PiSubmarine::Proximity::Telemetry::Api::State{
                    .Distance = Meters{0.5 + cycle}};
            }

        private:
            mutable int m_Tick = 0;
        };

        class FakeVideoProvider final : public ::PiSubmarine::Video::Telemetry::Api::IProvider
        {
        public:
            [[nodiscard]] Error::Api::Result<::PiSubmarine::Video::Telemetry::Api::Status> GetStatus() const override
            {
                ++m_Tick;
                const auto phase = (m_Tick / 20) % 3;
                if (phase == 0)
                {
                    return ::PiSubmarine::Video::Telemetry::Api::Status{
                        .IsStreamingEnabled = false,
                        .Subscribers = 0,
                        .Operational = ::PiSubmarine::Video::Telemetry::Api::OperationalState::Stopped,
                        .ActiveFaults = static_cast<::PiSubmarine::Video::Telemetry::Api::Faults>(0)};
                }

                if (phase == 1)
                {
                    return ::PiSubmarine::Video::Telemetry::Api::Status{
                        .IsStreamingEnabled = true,
                        .Subscribers = 1 + (m_Tick % 3),
                        .Operational = ::PiSubmarine::Video::Telemetry::Api::OperationalState::Streaming,
                        .ActiveFaults = static_cast<::PiSubmarine::Video::Telemetry::Api::Faults>(0)};
                }

                return ::PiSubmarine::Video::Telemetry::Api::Status{
                    .IsStreamingEnabled = true,
                    .Subscribers = 0,
                    .Operational = ::PiSubmarine::Video::Telemetry::Api::OperationalState::Faulted,
                    .ActiveFaults = ::PiSubmarine::Video::Telemetry::Api::Faults::NetworkError};
            }

        private:
            mutable int m_Tick = 0;
        };

        class FakeTimeProvider final : public ::PiSubmarine::Time::Telemetry::Api::IProvider
        {
        public:
            [[nodiscard]] Error::Api::Result<::PiSubmarine::Time::Telemetry::Api::State> GetState() const override
            {
                ++m_Tick;

                // Hold the same uptime for a while so the UI can demonstrate stale telemetry colors in fake mode.
                if (m_Tick % 220 < 110)
                {
                    m_Uptime += std::chrono::milliseconds{10};
                }

                m_SystemTime += std::chrono::milliseconds{10};
                return ::PiSubmarine::Time::Telemetry::Api::State{
                    .Uptime = m_Uptime,
                    .SystemTime = m_SystemTime};
            }

        private:
            mutable int m_Tick = 0;
            mutable std::chrono::nanoseconds m_Uptime{0};
            mutable ::PiSubmarine::Time::Telemetry::Api::SystemTimePoint m_SystemTime{
                std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now())};
        };
    }

    FakeProviders CreateFakeProviders(const std::size_t motorCount)
    {
        FakeProviders providers;
        providers.Ballast = std::make_unique<FakeBallastProvider>();
        providers.Lamp = std::make_unique<FakeLampProvider>();
        providers.Battery = std::make_unique<FakeBatteryProvider>();
        providers.Depth = std::make_unique<FakeDepthProvider>();
        providers.Motors.reserve(motorCount);
        providers.Proximity = std::make_unique<FakeProximityProvider>();
        providers.Time = std::make_unique<FakeTimeProvider>();
        providers.Video = std::make_unique<FakeVideoProvider>();

        for (std::size_t index = 0; index < motorCount; ++index)
        {
            providers.Motors.push_back(std::make_unique<FakeMotorProvider>(static_cast<int>(index) * 7));
        }

        return providers;
    }
}
