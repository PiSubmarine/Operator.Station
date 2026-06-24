#pragma once

#include <chrono>
#include <cstddef>
#include <vector>

#include "PiSubmarine/Input/Api/IAxis.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Operator::Station::Input
{
    class AxisMultiplexor final : public ::PiSubmarine::Input::Api::IAxis, public ::PiSubmarine::Time::ITickable
    {
    public:
        void SetSources(std::vector<::PiSubmarine::Input::Api::IAxis*> sources);

        [[nodiscard]] SignedNormalizedFraction GetValue() const override;
        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    private:
        [[nodiscard]] static SignedNormalizedFraction ReadValue(const ::PiSubmarine::Input::Api::IAxis* source);

        std::vector<::PiSubmarine::Input::Api::IAxis*> m_Sources;
        std::vector<SignedNormalizedFraction> m_LastValues;
        std::size_t m_AuthoritativeSourceIndex = 0;
        SignedNormalizedFraction m_Value = SignedNormalizedFraction(0.0);
    };
}
