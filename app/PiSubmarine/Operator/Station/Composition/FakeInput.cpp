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

    void FakeInput::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        m_Manager.Tick(uptime, deltaTime);
        m_Binder.Tick(uptime, deltaTime);
    }
}
