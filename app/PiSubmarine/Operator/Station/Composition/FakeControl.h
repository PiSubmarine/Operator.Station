#pragma once

#include "PiSubmarine/Operator/Station/Composition/IControl.h"
#include "PiSubmarine/Operator/Station/Control/FakeSink.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class FakeControl final : public IControl
    {
    public:
        [[nodiscard]] ::PiSubmarine::Control::Api::Input::ISink& GetSink() override;
        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    private:
        ::PiSubmarine::Operator::Station::Control::FakeSink m_Sink;
        bool m_HasLeaseState = false;
    };
}
