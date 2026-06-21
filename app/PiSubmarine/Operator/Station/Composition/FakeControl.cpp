#include "PiSubmarine/Operator/Station/Composition/FakeControl.h"

namespace PiSubmarine::Operator::Station::Composition
{
    ::PiSubmarine::Control::Api::Input::ISink& FakeControl::GetSink()
    {
        return m_Sink;
    }

    std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> FakeControl::GetTickables()
    {
        return {};
    }
}
