#pragma once

#include <chrono>
#include <cstddef>
#include <vector>

#include "PiSubmarine/Input/Api/IKey.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Operator::Station::Input
{
    class KeyMultiplexor final : public ::PiSubmarine::Input::Api::IKey, public ::PiSubmarine::Time::ITickable
    {
    public:
        void SetSources(std::vector<::PiSubmarine::Input::Api::IKey*> sources);

        [[nodiscard]] ::PiSubmarine::Input::Api::KeyState GetState() const override;
        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    private:
        [[nodiscard]] static ::PiSubmarine::Input::Api::KeyState ReadState(const ::PiSubmarine::Input::Api::IKey* source);

        std::vector<::PiSubmarine::Input::Api::IKey*> m_Sources;
        std::vector<::PiSubmarine::Input::Api::KeyState> m_LastStates;
        std::size_t m_AuthoritativeSourceIndex = 0;
        ::PiSubmarine::Input::Api::KeyState m_State = ::PiSubmarine::Input::Api::KeyState::Released;
    };
}
