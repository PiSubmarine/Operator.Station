#include "PiSubmarine/Operator/Station/Composition/FakeControl.h"

namespace PiSubmarine::Operator::Station::Composition
{
    ::PiSubmarine::Control::Api::Input::ISink& FakeControl::GetSink()
    {
        return m_Sink;
    }

    void FakeControl::Tick(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)
    {
        if (m_HasLeaseState)
        {
            return;
        }

        m_HasLeaseState = true;
        emit LeaseStateChanged(QStringLiteral("fake-control"));
    }
}
