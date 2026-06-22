#include "PiSubmarine/Operator/Station/Composition/RemoteControl.h"

#include <stdexcept>
#include <utility>

namespace PiSubmarine::Operator::Station::Composition
{
    RemoteControl::RemoteControl(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ::PiSubmarine::Udp::Api::Endpoint serverEndpoint,
        const std::size_t receiveQueueCapacity)
        : IControl()
        , m_Socket(receiveQueueCapacity)
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
    }

    ::PiSubmarine::Control::Api::Input::ISink& RemoteControl::GetSink()
    {
        return m_Client;
    }


    void RemoteControl::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        m_Socket.Tick(uptime, deltaTime);

        const auto leaseId = GetLeaseId();
        if (m_HasLeaseState && leaseId == m_LastLeaseId)
        {
            return;
        }

        m_LastLeaseId = leaseId;
        m_HasLeaseState = true;
        emit LeaseStateChanged(m_LastLeaseId);
    }

    OptionalLeaseId RemoteControl::GetLeaseId()
    {
        const auto lease = m_Client.GetLease();
        if (!lease.has_value())
        {
            return std::nullopt;
        }

        return QString::fromStdString(lease->Id.Value);
    }
}
