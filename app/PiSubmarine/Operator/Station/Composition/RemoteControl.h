#pragma once

#include <cstddef>
#include "PiSubmarine/Control/Client/Udp/Client.h"
#include "PiSubmarine/Control/Protobuf/Serializer.h"
#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Operator/Station/Composition/IControl.h"
#include "PiSubmarine/Security/Aead/Openssl/Provider.h"
#include "PiSubmarine/Security/Nonce/Openssl/Provider.h"
#include "PiSubmarine/Time/ITickable.h"
#include "PiSubmarine/Udp/Api/Endpoint.h"
#include "PiSubmarine/Udp/Asio/Socket.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class RemoteControl final : public IControl
    {
    public:
        RemoteControl(
            ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
            ::PiSubmarine::Udp::Api::Endpoint serverEndpoint,
            std::size_t receiveQueueCapacity);

        [[nodiscard]] ::PiSubmarine::Control::Api::Input::ISink& GetSink() override;
        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    private:
        [[nodiscard]] OptionalLeaseId GetLeaseId();

        ::PiSubmarine::Udp::Asio::Socket m_Socket;
        ::PiSubmarine::Control::Protobuf::Serializer m_Serializer;
        ::PiSubmarine::Security::Aead::Openssl::Provider m_AeadProvider;
        ::PiSubmarine::Security::Nonce::Openssl::Provider m_NonceProvider;
        ::PiSubmarine::Control::Client::Udp::Client m_Client;
        OptionalLeaseId m_LastLeaseId;
        bool m_HasLeaseState = false;
    };
}
