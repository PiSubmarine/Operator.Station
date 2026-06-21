#pragma once

#include <functional>
#include <vector>

#include "PiSubmarine/Operator/Station/Composition/IControl.h"
#include "PiSubmarine/Operator/Station/Control/FakeSink.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class FakeControl final : public IControl
    {
    public:
        [[nodiscard]] ::PiSubmarine::Control::Api::Input::ISink& GetSink() override;
        [[nodiscard]] std::vector<std::reference_wrapper<::PiSubmarine::Time::ITickable>> GetTickables() override;

    private:
        ::PiSubmarine::Operator::Station::Control::FakeSink m_Sink;
    };
}
