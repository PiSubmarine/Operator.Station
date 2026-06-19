#include <gtest/gtest.h>

#include <chrono>

#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/Operator/Station/Lease/IAsyncLeaseIssuer.h"
#include "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h"

namespace PiSubmarine::Operator::Station::Lease
{
    namespace
    {
        constexpr auto NotReadyCondition = static_cast<Error::Api::ErrorCondition>(3);

        class AsyncLeaseIssuerStub final : public IAsyncLeaseIssuer
        {
        public:
            [[nodiscard]] bool EnqueueAcquireLease(const ::PiSubmarine::Lease::Api::LeaseRequest& request) override
            {
                LastAcquireRequest = request;
                ++AcquireEnqueueCount;
                return true;
            }

            [[nodiscard]] std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>>
            TryTakeAcquireLeaseResult(const ::PiSubmarine::Lease::Api::LeaseRequest& request) override
            {
                LastAcquireRequest = request;
                auto result = std::move(NextAcquireResult);
                NextAcquireResult.reset();
                return result;
            }

            [[nodiscard]] bool EnqueueRenewLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override
            {
                LastRenewLeaseId = leaseId;
                ++RenewEnqueueCount;
                return true;
            }

            [[nodiscard]] std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::Lease>>
            TryTakeRenewLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override
            {
                LastRenewLeaseId = leaseId;
                auto result = std::move(NextRenewResult);
                NextRenewResult.reset();
                return result;
            }

            [[nodiscard]] bool EnqueueReleaseLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override
            {
                LastReleaseLeaseId = leaseId;
                ++ReleaseEnqueueCount;
                return true;
            }

            [[nodiscard]] std::optional<Error::Api::Result<void>>
            TryTakeReleaseLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override
            {
                LastReleaseLeaseId = leaseId;
                auto result = std::move(NextReleaseResult);
                NextReleaseResult.reset();
                return result;
            }

            std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>> NextAcquireResult;
            std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::Lease>> NextRenewResult;
            std::optional<Error::Api::Result<void>> NextReleaseResult;
            ::PiSubmarine::Lease::Api::LeaseRequest LastAcquireRequest{};
            ::PiSubmarine::Lease::Api::LeaseId LastRenewLeaseId{};
            ::PiSubmarine::Lease::Api::LeaseId LastReleaseLeaseId{};
            int AcquireEnqueueCount = 0;
            int RenewEnqueueCount = 0;
            int ReleaseEnqueueCount = 0;
        };
    }

    TEST(SyncLeaseIssuerProxyTest, AcquireLeaseReturnsNotReadyWhileAsyncRequestIsPending)
    {
        AsyncLeaseIssuerStub asyncLeaseIssuer;
        SyncLeaseIssuerProxy proxy(asyncLeaseIssuer);
        const auto request = ::PiSubmarine::Lease::Api::LeaseRequest{
            .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "video-main"}};

        const auto firstResult = proxy.AcquireLease(request);
        ASSERT_FALSE(firstResult.has_value());
        EXPECT_EQ(firstResult.error().Condition, NotReadyCondition);
        EXPECT_EQ(asyncLeaseIssuer.AcquireEnqueueCount, 1);

        const auto secondResult = proxy.AcquireLease(request);
        ASSERT_FALSE(secondResult.has_value());
        EXPECT_EQ(secondResult.error().Condition, NotReadyCondition);
        EXPECT_EQ(asyncLeaseIssuer.AcquireEnqueueCount, 1);
    }

    TEST(SyncLeaseIssuerProxyTest, AcquireLeaseCachesCompletedGrant)
    {
        AsyncLeaseIssuerStub asyncLeaseIssuer;
        SyncLeaseIssuerProxy proxy(asyncLeaseIssuer);
        const auto request = ::PiSubmarine::Lease::Api::LeaseRequest{
            .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "video-main"}};
        const auto leaseGrant = ::PiSubmarine::Lease::Api::LeaseGrant{
            .Lease = ::PiSubmarine::Lease::Api::Lease{
                .Id = ::PiSubmarine::Lease::Api::LeaseId{.Value = "lease-1"},
                .Resource = request.Resource,
                .Duration = std::chrono::seconds(4)}};

        ASSERT_FALSE(proxy.AcquireLease(request).has_value());
        asyncLeaseIssuer.NextAcquireResult = Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>(leaseGrant);

        const auto completedResult = proxy.AcquireLease(request);
        ASSERT_TRUE(completedResult.has_value());
        EXPECT_EQ(*completedResult, leaseGrant);

        const auto cachedResult = proxy.AcquireLease(request);
        ASSERT_TRUE(cachedResult.has_value());
        EXPECT_EQ(*cachedResult, leaseGrant);
    }
}
