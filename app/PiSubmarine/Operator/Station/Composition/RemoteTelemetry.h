#pragma once

#include <cstddef>
#include <memory>
#include "PiSubmarine/Ballast/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Battery/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Depth/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Lamp/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Motor/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Operator/Station/Composition/ITelemetry.h"
#include "PiSubmarine/Proximity/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Security/Aead/Openssl/Provider.h"
#include "PiSubmarine/Security/Nonce/Openssl/Provider.h"
#include "PiSubmarine/Telemetry/Client/Udp/Client.h"
#include "PiSubmarine/Telemetry/Client/Udp/Source.h"
#include "PiSubmarine/Time/ITickable.h"
#include "PiSubmarine/Time/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Udp/Api/Endpoint.h"
#include "PiSubmarine/Udp/Asio/Socket.h"
#include "PiSubmarine/Video/Telemetry/Protobuf/Deserializer.h"

namespace PiSubmarine::Lease::Api
{
    class ILeaseIssuer;
}

namespace PiSubmarine::Operator::Station::Composition
{
    class RemoteTelemetry final : public ITelemetry
    {
    public:
        RemoteTelemetry(
            ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
            ::PiSubmarine::Udp::Api::Endpoint serverEndpoint,
            std::size_t receiveQueueCapacity);

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
        [[nodiscard]] OptionalLeaseId GetLeaseId() const;

        ::PiSubmarine::Udp::Asio::Socket m_Socket;
        ::PiSubmarine::Security::Aead::Openssl::Provider m_AeadProvider;
        ::PiSubmarine::Security::Nonce::Openssl::Provider m_NonceProvider;
        ::PiSubmarine::Telemetry::Client::Udp::Client m_Client;

        ::PiSubmarine::Telemetry::Client::Udp::Source m_BallastSource;
        ::PiSubmarine::Telemetry::Client::Udp::Source m_BatterySource;
        ::PiSubmarine::Telemetry::Client::Udp::Source m_DepthSource;
        ::PiSubmarine::Telemetry::Client::Udp::Source m_LampSource;
        ::PiSubmarine::Telemetry::Client::Udp::Source m_BallastMotorSource;
        ::PiSubmarine::Telemetry::Client::Udp::Source m_ProximitySource;
        ::PiSubmarine::Telemetry::Client::Udp::Source m_TimeSource;
        ::PiSubmarine::Telemetry::Client::Udp::Source m_VideoSource;

        ::PiSubmarine::Ballast::Telemetry::Protobuf::Deserializer m_BallastProvider;
        ::PiSubmarine::Battery::Telemetry::Protobuf::Deserializer m_BatteryProvider;
        ::PiSubmarine::Depth::Telemetry::Protobuf::Deserializer m_DepthProvider;
        ::PiSubmarine::Lamp::Telemetry::Protobuf::Deserializer m_LampProvider;
        ::PiSubmarine::Motor::Telemetry::Protobuf::Deserializer m_BallastMotorProvider;
        ::PiSubmarine::Proximity::Telemetry::Protobuf::Deserializer m_ProximityProvider;
        ::PiSubmarine::Time::Telemetry::Protobuf::Deserializer m_TimeProvider;
        ::PiSubmarine::Video::Telemetry::Protobuf::Deserializer m_VideoProvider;

        std::vector<std::unique_ptr<::PiSubmarine::Telemetry::Client::Udp::Source>> m_MotorSources;
        std::vector<std::unique_ptr<::PiSubmarine::Motor::Telemetry::Protobuf::Deserializer>> m_MotorProvidersStorage;
        std::vector<std::reference_wrapper<::PiSubmarine::Motor::Telemetry::Api::IProvider>> m_Motors;
        OptionalLeaseId m_LastLeaseId;
        bool m_HasLeaseState = false;
    };
}
