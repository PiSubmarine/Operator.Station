#include "PiSubmarine/Operator/Station/Composition/RemoteTelemetry.h"

#include <array>
#include <stdexcept>
#include <string_view>

#include "PiSubmarine/Telemetry/Api/ChannelId.h"
#include "PiSubmarine/Telemetry/Channels/Api/Channels.h"

namespace PiSubmarine::Operator::Station::Composition
{
    namespace
    {
        [[nodiscard]] ::PiSubmarine::Telemetry::Api::ChannelId MakeChannelId(const std::string_view value)
        {
            return ::PiSubmarine::Telemetry::Api::ChannelId{.Value = std::string(value)};
        }

        constexpr std::array MotorChannelIds{
            ::PiSubmarine::Telemetry::Channels::Api::MotorFrontLeft,
            ::PiSubmarine::Telemetry::Channels::Api::MotorFrontRight,
            ::PiSubmarine::Telemetry::Channels::Api::MotorRearLeft,
            ::PiSubmarine::Telemetry::Channels::Api::MotorRearRight};
    }

    RemoteTelemetry::RemoteTelemetry(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Udp::Api::Endpoint serverEndpoint,
        const std::size_t receiveQueueCapacity)
        : ITelemetry()
        , m_Socket(receiveQueueCapacity)
        , m_Client(
            leaseIssuer,
            m_AeadProvider,
            m_NonceProvider,
            m_Socket,
            m_Socket,
            std::move(serverEndpoint))
        , m_BallastSource(m_Client, MakeChannelId(::PiSubmarine::Telemetry::Channels::Api::BallastMain))
        , m_BatterySource(m_Client, MakeChannelId(::PiSubmarine::Telemetry::Channels::Api::BatteryMain))
        , m_DepthSource(m_Client, MakeChannelId(::PiSubmarine::Telemetry::Channels::Api::DepthMain))
        , m_LampSource(m_Client, MakeChannelId(::PiSubmarine::Telemetry::Channels::Api::LampMain))
        , m_ProximitySource(m_Client, MakeChannelId(::PiSubmarine::Telemetry::Channels::Api::ProximityMain))
        , m_TimeSource(m_Client, MakeChannelId(::PiSubmarine::Telemetry::Channels::Api::TimeMain))
        , m_VideoSource(m_Client, MakeChannelId(::PiSubmarine::Telemetry::Channels::Api::VideoMain))
        , m_BallastProvider(m_BallastSource)
        , m_BatteryProvider(m_BatterySource)
        , m_DepthProvider(m_DepthSource)
        , m_LampProvider(m_LampSource)
        , m_ProximityProvider(m_ProximitySource)
        , m_TimeProvider(m_TimeSource)
        , m_VideoProvider(m_VideoSource)
    {
        const auto bindResult = m_Socket.Bind(::PiSubmarine::Udp::Api::Endpoint{
            .Address = "0.0.0.0",
            .Port = 0});
        if (!bindResult.has_value())
        {
            throw std::runtime_error("Failed to bind telemetry UDP socket");
        }

        m_MotorSources.reserve(MotorChannelIds.size());
        m_MotorProvidersStorage.reserve(MotorChannelIds.size());
        m_Motors.reserve(MotorChannelIds.size());
        for (const auto channelId : MotorChannelIds)
        {
            m_MotorSources.push_back(std::make_unique<::PiSubmarine::Telemetry::Client::Udp::Source>(
                m_Client,
                MakeChannelId(channelId)));
            m_MotorProvidersStorage.push_back(
                std::make_unique<::PiSubmarine::Motor::Telemetry::Protobuf::Deserializer>(*m_MotorSources.back()));
            m_Motors.emplace_back(*m_MotorProvidersStorage.back());
        }

    }

    ::PiSubmarine::Ballast::Telemetry::Api::IProvider& RemoteTelemetry::GetBallast()
    {
        return m_BallastProvider;
    }

    ::PiSubmarine::Battery::Telemetry::Api::IProvider& RemoteTelemetry::GetBattery()
    {
        return m_BatteryProvider;
    }

    ::PiSubmarine::Depth::Telemetry::Api::IProvider& RemoteTelemetry::GetDepth()
    {
        return m_DepthProvider;
    }

    ::PiSubmarine::Lamp::Telemetry::Api::IProvider& RemoteTelemetry::GetLamp()
    {
        return m_LampProvider;
    }

    std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>> RemoteTelemetry::GetMotors()
    {
        return m_Motors;
    }

    ::PiSubmarine::Proximity::Telemetry::Api::IProvider& RemoteTelemetry::GetProximity()
    {
        return m_ProximityProvider;
    }

    ::PiSubmarine::Time::Telemetry::Api::IProvider& RemoteTelemetry::GetTime()
    {
        return m_TimeProvider;
    }

    ::PiSubmarine::Video::Telemetry::Api::IProvider& RemoteTelemetry::GetVideo()
    {
        return m_VideoProvider;
    }

    void RemoteTelemetry::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        m_Socket.Tick(uptime, deltaTime);
        m_Client.Tick(uptime, deltaTime);

        const auto leaseId = GetLeaseId();
        if (m_HasLeaseState && leaseId == m_LastLeaseId)
        {
            return;
        }

        m_LastLeaseId = leaseId;
        m_HasLeaseState = true;
        emit LeaseStateChanged(m_LastLeaseId);
    }

    OptionalLeaseId RemoteTelemetry::GetLeaseId() const
    {
        const auto lease = m_Client.GetLease();
        if (!lease.has_value())
        {
            return std::nullopt;
        }

        return QString::fromStdString(lease->Id.Value);
    }
}
