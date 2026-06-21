#include "PiSubmarine/Operator/Station/Composition/FakeInput.h"

namespace PiSubmarine::Operator::Station::Composition
{
    ::PiSubmarine::Input::Api::IManager& FakeInput::GetManager()
    {
        return m_Manager;
    }

    ::PiSubmarine::Input::Api::IBinder& FakeInput::GetBinder()
    {
        return m_Binder;
    }

    ::PiSubmarine::Input::Api::ISerializer& FakeInput::GetSerializer()
    {
        return m_Serializer;
    }

    std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> FakeInput::GetTickables()
    {
        return {m_Manager, m_Binder};
    }
}
