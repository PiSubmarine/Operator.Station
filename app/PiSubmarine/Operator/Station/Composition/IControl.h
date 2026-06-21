#pragma once

#include <functional>
#include <vector>

#include "PiSubmarine/Control/Api/Input/ISink.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class IControl
    {
    public:
        virtual ~IControl() = default;

        [[nodiscard]] virtual ::PiSubmarine::Control::Api::Input::ISink& GetSink() = 0;
        [[nodiscard]] virtual std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> GetTickables() = 0;
    };
}
