#include "PiSubmarine/Operator/Station/Lease/FakeIssuer.h"

#include <chrono>
#include <string>

namespace PiSubmarine::Operator::Station::Lease
{
    Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant> FakeIssuer::AcquireLease(
        const ::PiSubmarine::Lease::Api::LeaseRequest& request)
    {
        std::lock_guard lock(m_Mutex);
        ++m_Counter;

        return ::PiSubmarine::Lease::Api::LeaseGrant{
            .Lease = ::PiSubmarine::Lease::Api::Lease{
                .Id = ::PiSubmarine::Lease::Api::LeaseId{.Value = "operator-station-lease-" + std::to_string(m_Counter)},
                .Resource = request.Resource,
                .Duration = std::chrono::seconds(4)}};
    }

    Error::Api::Result<::PiSubmarine::Lease::Api::Lease> FakeIssuer::RenewLease(
        const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        return ::PiSubmarine::Lease::Api::Lease{
            .Id = leaseId,
            .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "operator-station-resource"},
            .Duration = std::chrono::seconds(4)};
    }

    Error::Api::Result<void> FakeIssuer::ReleaseLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        static_cast<void>(leaseId);
        return {};
    }
}
