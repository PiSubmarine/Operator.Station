#pragma once

#include <mutex>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"

namespace PiSubmarine::Operator::Station::Lease
{
    class FakeIssuer final : public ::PiSubmarine::Lease::Api::ILeaseIssuer
    {
    public:
        [[nodiscard]] Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant> AcquireLease(
            const ::PiSubmarine::Lease::Api::LeaseRequest& request) override;
        [[nodiscard]] Error::Api::Result<::PiSubmarine::Lease::Api::Lease> RenewLease(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;
        [[nodiscard]] Error::Api::Result<void> ReleaseLease(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;

    private:
        std::mutex m_Mutex;
        int m_Counter = 0;
    };
}
