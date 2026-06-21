#pragma once

#include <memory>

#include "PiSubmarine/Operator/Station/Composition/ILease.h"
#include "PiSubmarine/Operator/Station/Lease/FakeIssuer.h"
#include "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h"
#include "PiSubmarine/Operator/Station/Lease/ThreadWorker.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class FakeLease final : public ILease
    {
    public:
        explicit FakeLease(PiSubmarine::Logging::Api::IFactory& loggerFactory);

        [[nodiscard]] ::PiSubmarine::Lease::Api::ILeaseIssuer& GetIssuer() override;
        [[nodiscard]] QObject& GetWorkerObject() override;

    private:
        ::PiSubmarine::Operator::Station::Lease::FakeIssuer m_BlockingIssuer;
        std::unique_ptr<::PiSubmarine::Operator::Station::Lease::ThreadWorker> m_Worker;
        std::unique_ptr<::PiSubmarine::Operator::Station::Lease::SyncLeaseIssuerProxy> m_Issuer;
    };
}
