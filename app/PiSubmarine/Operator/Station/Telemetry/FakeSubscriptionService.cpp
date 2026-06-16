#include "PiSubmarine/Operator/Station/Telemetry/FakeSubscriptionService.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    Error::Api::Result<void> FakeSubscriptionService::Subscribe(
        const ::PiSubmarine::Lease::Api::LeaseId& leaseId,
        const Shared::Endpoint& endpoint)
    {
        m_LastLeaseId = leaseId;
        m_LastEndpoint = endpoint;
        return {};
    }

    Error::Api::Result<void> FakeSubscriptionService::Unsubscribe(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        m_LastLeaseId = leaseId;
        return {};
    }
}
