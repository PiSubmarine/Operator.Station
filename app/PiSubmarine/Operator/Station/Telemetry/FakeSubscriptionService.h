#pragma once

#include "PiSubmarine/Operator/Station/Telemetry/ISubscriptionService.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class FakeSubscriptionService final : public ISubscriptionService
    {
    public:
        [[nodiscard]] Error::Api::Result<void> Subscribe(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId,
            const Shared::Endpoint& endpoint) override;
        [[nodiscard]] Error::Api::Result<void> Unsubscribe(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;

    private:
        ::PiSubmarine::Lease::Api::LeaseId m_LastLeaseId{};
        Shared::Endpoint m_LastEndpoint{};
    };
}
