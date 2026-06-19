#include "PiSubmarine/Operator/Station/Lease/ThreadWorker.h"

#include <stdexcept>
#include <utility>

#include <QMetaObject>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Error/Api/ErrorCondition.h"
#include "PiSubmarine/Error/Api/MakeError.h"

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

    ThreadWorker::ThreadWorker(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_LeaseIssuer(leaseIssuer)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.LeaseWorker"))
    {
        if (!m_Logger)
        {
            throw std::invalid_argument("Operator.Station.LeaseWorker requires a logger");
        }
    }

    bool ThreadWorker::EnqueueAcquireLease(const ::PiSubmarine::Lease::Api::LeaseRequest& request)
    {
        const auto key = MakeResourceKey(request);

        {
            std::lock_guard lock(m_Mutex);
            auto& state = m_AcquireStates[key];
            if (state.IsQueued || state.IsInFlight || state.Completed.has_value())
            {
                return false;
            }

            state.Request = request;
            state.IsQueued = true;
        }

        return ScheduleProcessPending();
    }

    Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>
    ThreadWorker::TryTakeAcquireLeaseResult(const ::PiSubmarine::Lease::Api::LeaseRequest& request)
    {
        const auto key = MakeResourceKey(request);
        std::lock_guard lock(m_Mutex);

        const auto iterator = m_AcquireStates.find(key);
        if (iterator == m_AcquireStates.end() || !iterator->second.Completed.has_value())
        {
            return std::unexpected(MakeNotReadyError());
        }

        auto result = std::move(iterator->second.Completed);
        m_AcquireStates.erase(iterator);
        return std::move(*result);
    }

    bool ThreadWorker::EnqueueRenewLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        const auto key = MakeLeaseKey(leaseId);

        {
            std::lock_guard lock(m_Mutex);
            auto& state = m_RenewStates[key];
            if (state.IsQueued || state.IsInFlight || state.Completed.has_value())
            {
                return false;
            }

            state.Request = leaseId;
            state.IsQueued = true;
        }

        return ScheduleProcessPending();
    }

    Error::Api::Result<::PiSubmarine::Lease::Api::Lease>
    ThreadWorker::TryTakeRenewLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        const auto key = MakeLeaseKey(leaseId);
        std::lock_guard lock(m_Mutex);

        const auto iterator = m_RenewStates.find(key);
        if (iterator == m_RenewStates.end() || !iterator->second.Completed.has_value())
        {
            return std::unexpected(MakeNotReadyError());
        }

        auto result = std::move(iterator->second.Completed);
        m_RenewStates.erase(iterator);
        return std::move(*result);
    }

    bool ThreadWorker::EnqueueReleaseLease(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        const auto key = MakeLeaseKey(leaseId);

        {
            std::lock_guard lock(m_Mutex);
            auto& state = m_ReleaseStates[key];
            if (state.IsQueued || state.IsInFlight || state.Completed.has_value())
            {
                return false;
            }

            state.Request = leaseId;
            state.IsQueued = true;
        }

        return ScheduleProcessPending();
    }

    Error::Api::Result<void>
    ThreadWorker::TryTakeReleaseLeaseResult(const ::PiSubmarine::Lease::Api::LeaseId& leaseId)
    {
        const auto key = MakeLeaseKey(leaseId);
        std::lock_guard lock(m_Mutex);

        const auto iterator = m_ReleaseStates.find(key);
        if (iterator == m_ReleaseStates.end() || !iterator->second.Completed.has_value())
        {
            return std::unexpected(MakeNotReadyError());
        }

        auto result = std::move(iterator->second.Completed);
        m_ReleaseStates.erase(iterator);
        return std::move(*result);
    }

    void ThreadWorker::ProcessPending()
    {
        for (;;)
        {
            std::optional<std::pair<std::string, ::PiSubmarine::Lease::Api::LeaseRequest>> acquireRequest;
            std::optional<std::pair<std::string, ::PiSubmarine::Lease::Api::LeaseId>> renewRequest;
            std::optional<std::pair<std::string, ::PiSubmarine::Lease::Api::LeaseId>> releaseRequest;

            {
                std::lock_guard lock(m_Mutex);

                for (auto& [key, state] : m_AcquireStates)
                {
                    if (state.IsQueued && !state.IsInFlight)
                    {
                        state.IsQueued = false;
                        state.IsInFlight = true;
                        acquireRequest = std::pair{key, state.Request};
                        break;
                    }
                }

                if (!acquireRequest.has_value())
                {
                    for (auto& [key, state] : m_RenewStates)
                    {
                        if (state.IsQueued && !state.IsInFlight)
                        {
                            state.IsQueued = false;
                            state.IsInFlight = true;
                            renewRequest = std::pair{key, state.Request};
                            break;
                        }
                    }
                }

                if (!acquireRequest.has_value() && !renewRequest.has_value())
                {
                    for (auto& [key, state] : m_ReleaseStates)
                    {
                        if (state.IsQueued && !state.IsInFlight)
                        {
                            state.IsQueued = false;
                            state.IsInFlight = true;
                            releaseRequest = std::pair{key, state.Request};
                            break;
                        }
                    }
                }
            }

            if (acquireRequest.has_value())
            {
                SPDLOG_LOGGER_DEBUG(m_Logger, "Acquiring lease for '{}'", acquireRequest->second.Resource.Value);
                auto result = m_LeaseIssuer.AcquireLease(acquireRequest->second);
                std::lock_guard lock(m_Mutex);
                auto& state = m_AcquireStates[acquireRequest->first];
                state.IsInFlight = false;
                state.Completed = std::move(result);
                continue;
            }

            if (renewRequest.has_value())
            {
                SPDLOG_LOGGER_DEBUG(m_Logger, "Renewing lease '{}'", renewRequest->second.Value);
                auto result = m_LeaseIssuer.RenewLease(renewRequest->second);
                std::lock_guard lock(m_Mutex);
                auto& state = m_RenewStates[renewRequest->first];
                state.IsInFlight = false;
                state.Completed = std::move(result);
                continue;
            }

            if (releaseRequest.has_value())
            {
                SPDLOG_LOGGER_DEBUG(m_Logger, "Releasing lease '{}'", releaseRequest->second.Value);
                auto result = m_LeaseIssuer.ReleaseLease(releaseRequest->second);
                std::lock_guard lock(m_Mutex);
                auto& state = m_ReleaseStates[releaseRequest->first];
                state.IsInFlight = false;
                state.Completed = std::move(result);
                continue;
            }

            break;
        }
    }

    bool ThreadWorker::ScheduleProcessPending()
    {
        return QMetaObject::invokeMethod(this, &ThreadWorker::ProcessPending, Qt::QueuedConnection);
    }

    Error::Api::Error ThreadWorker::MakeNotReadyError()
    {
        return Error::Api::MakeError(Error::Api::ErrorCondition::NotReady);
    }
}
