#pragma once

#include <mutex>
#include <unordered_map>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Operator/Station/Lease/IAsyncLeaseIssuer.h"

namespace PiSubmarine::Operator::Station::Lease
{
    class SyncLeaseIssuerProxy final : public ::PiSubmarine::Lease::Api::ILeaseIssuer
    {
    public:
        explicit SyncLeaseIssuerProxy(IAsyncLeaseIssuer& asyncLeaseIssuer);

        [[nodiscard]] Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant> AcquireLease(
            const ::PiSubmarine::Lease::Api::LeaseRequest& request) override;
        [[nodiscard]] Error::Api::Result<::PiSubmarine::Lease::Api::Lease> RenewLease(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;
        [[nodiscard]] Error::Api::Result<void> ReleaseLease(
            const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;

    private:
        [[nodiscard]] static Error::Api::ErrorCondition GetNotReadyCondition();
        [[nodiscard]] static Error::Api::Error MakeNotReadyError();

        IAsyncLeaseIssuer& m_AsyncLeaseIssuer;
        std::mutex m_Mutex;
        std::unordered_map<std::string, ::PiSubmarine::Lease::Api::LeaseGrant> m_CachedGrantsByResource;
        std::unordered_map<std::string, ::PiSubmarine::Lease::Api::Lease> m_CachedLeasesById;
        std::unordered_map<std::string, ::PiSubmarine::Lease::Api::LeaseRequest> m_PendingAcquireRequests;
        std::unordered_map<std::string, ::PiSubmarine::Lease::Api::LeaseId> m_PendingRenewRequests;
        std::unordered_map<std::string, ::PiSubmarine::Lease::Api::LeaseId> m_PendingReleaseRequests;
    };
}
