#include "PiSubmarine/Operator/Station/Composition/FakeLease.h"

namespace PiSubmarine::Operator::Station::Composition
{
    FakeLease::FakeLease(PiSubmarine::Logging::Api::IFactory& loggerFactory)
        : m_Worker(std::make_unique<::PiSubmarine::Operator::Station::Lease::ThreadWorker>(
            m_BlockingIssuer,
            loggerFactory))
        , m_Issuer(std::make_unique<::PiSubmarine::Operator::Station::Lease::SyncLeaseIssuerProxy>(*m_Worker))
    {
    }

    ::PiSubmarine::Lease::Api::ILeaseIssuer& FakeLease::GetIssuer()
    {
        return *m_Issuer;
    }

    QObject& FakeLease::GetWorkerObject()
    {
        return *m_Worker;
    }
}
