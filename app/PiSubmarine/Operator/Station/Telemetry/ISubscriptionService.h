#pragma once

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Lease/Api/Identifiers.h"
#include "PiSubmarine/Operator/Station/Shared/Endpoint.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    // FIXME PiSubmarine::Operator::Station::Telemetry::ISubscriptionService is not needed. Subscription is maintained internally by Telemetry.Client.Udp. Remove ISubscriptionService type completely.
    class ISubscriptionService
    {
    public:
        virtual ~ISubscriptionService() = default;

        [[nodiscard]] virtual Error::Api::Result<void> Subscribe(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId,
            const Shared::Endpoint& endpoint) = 0;
        [[nodiscard]] virtual Error::Api::Result<void> Unsubscribe(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId) = 0;
    };
}
