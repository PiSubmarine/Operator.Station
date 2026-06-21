#include "PiSubmarine/Operator/Station/Composition/RemoteControl.h"

#include <stdexcept>
#include <utility>

namespace PiSubmarine::Operator::Station::Composition
{
    RemoteControl::RemoteControl(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Udp::Api::Endpoint serverEndpoint,
        const std::size_t receiveQueueCapacity)
        : m_Socket(receiveQueueCapacity)
        , m_Client(
            leaseIssuer,
            m_Serializer,
            m_AeadProvider,
            m_NonceProvider,
            m_Socket,
            std::move(serverEndpoint))
    {
        const auto bindResult = m_Socket.Bind(::PiSubmarine::Udp::Api::Endpoint{
            .Address = "0.0.0.0",
            .Port = 0});
        if (!bindResult.has_value())
        {
            throw std::runtime_error("Failed to bind control UDP socket");
        }

        m_Tickables.emplace_back(m_Socket);
    }

    ::PiSubmarine::Control::Api::Input::ISink& RemoteControl::GetSink()
    {
        return m_Client;
    }

    std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> RemoteControl::GetTickables()
    {
        return m_Tickables;
    }
}
