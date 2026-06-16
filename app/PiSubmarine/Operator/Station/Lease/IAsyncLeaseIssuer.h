#pragma once

#include <optional>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Lease/Api/Lease.h"
#include "PiSubmarine/Lease/Api/LeaseGrant.h"
#include "PiSubmarine/Lease/Api/LeaseRequest.h"

namespace PiSubmarine::Operator::Station::Lease
{
    class IAsyncLeaseIssuer
    {
    public:
        virtual ~IAsyncLeaseIssuer() = default;

        [[nodiscard]] virtual bool EnqueueAcquireLease(const ::PiSubmarine::Lease::Api::LeaseRequest& request) = 0;
        [[nodiscard]] virtual std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>>
        TryTakeAcquireLeaseResult(const ::PiSubmarine::Lease::Api::LeaseRequest& request) = 0;

        [[nodiscard]] virtual bool EnqueueRenewLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) = 0;
        [[nodiscard]] virtual std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::Lease>>
        TryTakeRenewLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) = 0;

        [[nodiscard]] virtual bool EnqueueReleaseLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) = 0;
        [[nodiscard]] virtual std::optional<Error::Api::Result<void>>
        TryTakeReleaseLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) = 0;
    };
}
