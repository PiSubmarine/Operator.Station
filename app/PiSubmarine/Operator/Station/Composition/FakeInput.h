#pragma once

#include <functional>
#include <vector>

#include "PiSubmarine/Input/Fake/Binder.h"
#include "PiSubmarine/Input/Fake/Manager.h"
#include "PiSubmarine/Input/Fake/Serializer.h"
#include "PiSubmarine/Operator/Station/Composition/IInput.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class FakeInput final : public IInput
    {
    public:
        [[nodiscard]] ::PiSubmarine::Input::Api::IManager& GetManager() override;
        [[nodiscard]] ::PiSubmarine::Input::Api::IBinder& GetBinder() override;
        [[nodiscard]] ::PiSubmarine::Input::Api::ISerializer& GetSerializer() override;
        [[nodiscard]] std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> GetTickables() override;

    private:
        ::PiSubmarine::Input::Fake::Manager m_Manager;
        ::PiSubmarine::Input::Fake::Binder m_Binder;
        ::PiSubmarine::Input::Fake::Serializer m_Serializer;
    };
}
