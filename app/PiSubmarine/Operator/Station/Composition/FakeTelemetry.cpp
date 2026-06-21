#include "PiSubmarine/Operator/Station/Composition/FakeTelemetry.h"

namespace PiSubmarine::Operator::Station::Composition
{
    FakeTelemetry::FakeTelemetry(const std::size_t motorCount)
        : m_Providers(Telemetry::CreateFakeProviders(motorCount))
    {
        m_Motors.reserve(m_Providers.Motors.size());
        for (const auto& motor : m_Providers.Motors)
        {
            m_Motors.emplace_back(*motor);
        }
    }

    ::PiSubmarine::Ballast::Telemetry::Api::IProvider& FakeTelemetry::GetBallast()
    {
        return *m_Providers.Ballast;
    }

    ::PiSubmarine::Battery::Telemetry::Api::IProvider& FakeTelemetry::GetBattery()
    {
        return *m_Providers.Battery;
    }

    ::PiSubmarine::Depth::Telemetry::Api::IProvider& FakeTelemetry::GetDepth()
    {
        return *m_Providers.Depth;
    }

    ::PiSubmarine::Lamp::Telemetry::Api::IProvider& FakeTelemetry::GetLamp()
    {
        return *m_Providers.Lamp;
    }

    std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>> FakeTelemetry::GetMotors()
    {
        return m_Motors;
    }

    ::PiSubmarine::Proximity::Telemetry::Api::IProvider& FakeTelemetry::GetProximity()
    {
        return *m_Providers.Proximity;
    }

    ::PiSubmarine::Time::Telemetry::Api::IProvider& FakeTelemetry::GetTime()
    {
        return *m_Providers.Time;
    }

    ::PiSubmarine::Video::Telemetry::Api::IProvider& FakeTelemetry::GetVideo()
    {
        return *m_Providers.Video;
    }

    bool FakeTelemetry::HasLease() const
    {
        return true;
    }

    std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> FakeTelemetry::GetTickables()
    {
        return {};
    }
}
