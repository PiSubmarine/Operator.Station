#include "PiSubmarine/Operator/Station/Input/KeyMultiplexor.h"

namespace PiSubmarine::Operator::Station::Input
{
    void KeyMultiplexor::SetSources(std::vector<::PiSubmarine::Input::Api::IKey*> sources)
    {
        const auto* currentAuthoritativeSource =
            m_Sources.empty() || m_AuthoritativeSourceIndex >= m_Sources.size()
                ? nullptr
                : m_Sources.at(m_AuthoritativeSourceIndex);

        m_Sources = std::move(sources);
        m_LastStates.clear();
        m_LastStates.reserve(m_Sources.size());

        std::size_t nextAuthoritativeSourceIndex = 0;
        bool hasAuthoritativeSource = false;
        for (std::size_t index = 0; index < m_Sources.size(); ++index)
        {
            m_LastStates.push_back(ReadState(m_Sources.at(index)));
            if (m_Sources.at(index) == currentAuthoritativeSource)
            {
                nextAuthoritativeSourceIndex = index;
                hasAuthoritativeSource = true;
            }
        }

        if (!hasAuthoritativeSource)
        {
            nextAuthoritativeSourceIndex = 0;
        }

        m_AuthoritativeSourceIndex = nextAuthoritativeSourceIndex;
        m_State = m_Sources.empty() ? ::PiSubmarine::Input::Api::KeyState::Released : m_LastStates.at(m_AuthoritativeSourceIndex);
    }

    ::PiSubmarine::Input::Api::KeyState KeyMultiplexor::GetState() const
    {
        return m_State;
    }

    void KeyMultiplexor::Tick(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)
    {
        if (m_Sources.empty())
        {
            m_State = ::PiSubmarine::Input::Api::KeyState::Released;
            return;
        }

        for (std::size_t index = 0; index < m_Sources.size(); ++index)
        {
            const auto currentState = ReadState(m_Sources.at(index));
            if (currentState != m_LastStates.at(index))
            {
                m_LastStates.at(index) = currentState;
                m_AuthoritativeSourceIndex = index;
            }
        }

        m_State = m_LastStates.at(m_AuthoritativeSourceIndex);
    }

    ::PiSubmarine::Input::Api::KeyState KeyMultiplexor::ReadState(const ::PiSubmarine::Input::Api::IKey* source)
    {
        return source == nullptr ? ::PiSubmarine::Input::Api::KeyState::Released : source->GetState();
    }
}
