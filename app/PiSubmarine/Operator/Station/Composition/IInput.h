#pragma once

#include "PiSubmarine/Input/Api/IBinder.h"
#include "PiSubmarine/Input/Api/IManager.h"
#include "PiSubmarine/Input/Api/ISerializer.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class IInput : public ::PiSubmarine::Time::ITickable
    {
    public:
        virtual ~IInput() = default;

        [[nodiscard]] virtual ::PiSubmarine::Input::Api::IManager& GetManager() = 0;
        [[nodiscard]] virtual ::PiSubmarine::Input::Api::IBinder& GetBinder() = 0;
        [[nodiscard]] virtual ::PiSubmarine::Input::Api::ISerializer& GetSerializer() = 0;
    };
}
