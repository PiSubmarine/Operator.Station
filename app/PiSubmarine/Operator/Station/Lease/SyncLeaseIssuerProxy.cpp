#include "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h"

namespace PiSubmarine::Operator::Station::Lease
{
    namespace
    {
        [[nodiscard]] std::string MakeResourceKey(const ::PiSubmarine::Lease::Api::LeaseRequest& request)
        {
            return request.Resource.Value;
        }

        [[nodiscard]] std::string MakeLeaseKey(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
        {
            return leaseId.Value;
        }
    }

    SyncLeaseIssuerProxy::SyncLeaseIssuerProxy(IAsyncLeaseIssuer& asyncLeaseIssuer)
        : m_AsyncLeaseIssuer(asyncLeaseIssuer)
    {
    }

    Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant> SyncLeaseIssuerProxy::AcquireLease(
        const ::PiSubmarine::Lease::Api::LeaseRequest& request)
    {
        const auto key = MakeResourceKey(request);

        std::lock_guard lock(m_Mutex);

        if (auto readyResult = m_AsyncLeaseIssuer.TryTakeAcquireLeaseResult(request); readyResult.has_value())
        {
            m_PendingAcquireRequests.erase(key);
            if (readyResult->has_value())
            {
                m_CachedGrantsByResource[key] = **readyResult;
                m_CachedLeasesById[(**readyResult).Lease.Id.Value] = (**readyResult).Lease;
                return **readyResult;
            }

            return std::unexpected(readyResult->error());
        }

        if (const auto cachedGrant = m_CachedGrantsByResource.find(key); cachedGrant != m_CachedGrantsByResource.end())
        {
            return cachedGrant->second;
        }

        if (m_PendingAcquireRequests.contains(key))
        {
            return std::unexpected(MakeNotReadyError());
        }

        if (m_AsyncLeaseIssuer.EnqueueAcquireLease(request))
        {
            m_PendingAcquireRequests.emplace(key, request);
        }

        return std::unexpected(MakeNotReadyError());
    }

    Error::Api::Result<::PiSubmarine::Lease::Api::Lease> SyncLeaseIssuerProxy::RenewLease(
        const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        const auto key = MakeLeaseKey(leaseId);

        std::lock_guard lock(m_Mutex);

        if (auto readyResult = m_AsyncLeaseIssuer.TryTakeRenewLeaseResult(leaseId); readyResult.has_value())
        {
            m_PendingRenewRequests.erase(key);
            if (readyResult->has_value())
            {
                m_CachedLeasesById[key] = **readyResult;
                return **readyResult;
            }

            return std::unexpected(readyResult->error());
        }

        const auto cachedLease = m_CachedLeasesById.find(key);
        if (cachedLease != m_CachedLeasesById.end())
        {
            if (!m_PendingRenewRequests.contains(key) && m_AsyncLeaseIssuer.EnqueueRenewLease(leaseId))
            {
                m_PendingRenewRequests.emplace(key, leaseId);
            }

            return cachedLease->second;
        }

        if (m_PendingRenewRequests.contains(key))
        {
            return std::unexpected(MakeNotReadyError());
        }

        if (m_AsyncLeaseIssuer.EnqueueRenewLease(leaseId))
        {
            m_PendingRenewRequests.emplace(key, leaseId);
        }

        return std::unexpected(MakeNotReadyError());
    }

    Error::Api::Result<void> SyncLeaseIssuerProxy::ReleaseLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        const auto key = MakeLeaseKey(leaseId);

        std::lock_guard lock(m_Mutex);

        if (auto readyResult = m_AsyncLeaseIssuer.TryTakeReleaseLeaseResult(leaseId); readyResult.has_value())
        {
            m_PendingReleaseRequests.erase(key);
            m_CachedLeasesById.erase(key);

            for (auto iterator = m_CachedGrantsByResource.begin(); iterator != m_CachedGrantsByResource.end();)
            {
                if (iterator->second.Lease.Id == leaseId)
                {
                    iterator = m_CachedGrantsByResource.erase(iterator);
                    continue;
                }

                ++iterator;
            }

            return *readyResult;
        }

        if (m_PendingReleaseRequests.contains(key))
        {
            return std::unexpected(MakeNotReadyError());
        }

        m_CachedLeasesById.erase(key);
        for (auto iterator = m_CachedGrantsByResource.begin(); iterator != m_CachedGrantsByResource.end();)
        {
            if (iterator->second.Lease.Id == leaseId)
            {
                iterator = m_CachedGrantsByResource.erase(iterator);
                continue;
            }

            ++iterator;
        }

        if (m_AsyncLeaseIssuer.EnqueueReleaseLease(leaseId))
        {
            m_PendingReleaseRequests.emplace(key, leaseId);
        }

        return std::unexpected(MakeNotReadyError());
    }

    Error::Api::Error SyncLeaseIssuerProxy::MakeNotReadyError()
    {
        return Error::Api::MakeError(GetNotReadyCondition());
    }

    Error::Api::ErrorCondition SyncLeaseIssuerProxy::GetNotReadyCondition()
    {
        return static_cast<Error::Api::ErrorCondition>(3);
    }
}
