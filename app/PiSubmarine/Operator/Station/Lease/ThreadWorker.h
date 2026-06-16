#pragma once

#include <mutex>
#include <optional>
#include <unordered_map>

#include <QObject>

#include <spdlog/logger.h>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Lease/IAsyncLeaseIssuer.h"

namespace PiSubmarine::Operator::Station::Lease
{
    class ThreadWorker final : public QObject, public IAsyncLeaseIssuer
    {
        Q_OBJECT

    public:
        ThreadWorker(
            ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);

        [[nodiscard]] bool EnqueueAcquireLease(const ::PiSubmarine::Lease::Api::LeaseRequest& request) override;
        [[nodiscard]] std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>>
        TryTakeAcquireLeaseResult(const ::PiSubmarine::Lease::Api::LeaseRequest& request) override;

        [[nodiscard]] bool EnqueueRenewLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;
        [[nodiscard]] std::optional<Error::Api::Result<::PiSubmarine::Lease::Api::Lease>>
        TryTakeRenewLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;

        [[nodiscard]] bool EnqueueReleaseLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;
        [[nodiscard]] std::optional<Error::Api::Result<void>>
        TryTakeReleaseLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId) override;

    private slots:
        void ProcessPending();

    private:
        template<typename TRequest, typename TResult>
        struct RequestState
        {
            TRequest Request;
            bool IsQueued = false;
            bool IsInFlight = false;
            std::optional<Error::Api::Result<TResult>> Completed;
        };

        using AcquireState = RequestState<::PiSubmarine::Lease::Api::LeaseRequest, ::PiSubmarine::Lease::Api::LeaseGrant>;
        using RenewState = RequestState<::PiSubmarine::Lease::Api::LeaseId, ::PiSubmarine::Lease::Api::Lease>;
        using ReleaseState = RequestState<::PiSubmarine::Lease::Api::LeaseId, void>;

        [[nodiscard]] bool ScheduleProcessPending();

        ::PiSubmarine::Lease::Api::ILeaseIssuer& m_LeaseIssuer;
        std::shared_ptr<spdlog::logger> m_Logger;
        std::mutex m_Mutex;
        std::unordered_map<std::string, AcquireState> m_AcquireStates;
        std::unordered_map<std::string, RenewState> m_RenewStates;
        std::unordered_map<std::string, ReleaseState> m_ReleaseStates;
    };
}
