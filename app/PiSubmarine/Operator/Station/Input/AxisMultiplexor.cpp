#include "PiSubmarine/Operator/Station/Input/AxisMultiplexor.h"

namespace PiSubmarine::Operator::Station::Input
{
    void AxisMultiplexor::SetSources(std::vector<::PiSubmarine::Input::Api::IAxis*> sources)
    {
        const auto* currentAuthoritativeSource =
            m_Sources.empty() || m_AuthoritativeSourceIndex >= m_Sources.size()
                ? nullptr
                : m_Sources.at(m_AuthoritativeSourceIndex);

        m_Sources = std::move(sources);
        m_LastValues.clear();
        m_LastValues.reserve(m_Sources.size());

        std::size_t nextAuthoritativeSourceIndex = 0;
        bool hasAuthoritativeSource = false;
        for (std::size_t index = 0; index < m_Sources.size(); ++index)
        {
            m_LastValues.push_back(ReadValue(m_Sources.at(index)));
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
        m_Value = m_Sources.empty() ? SignedNormalizedFraction(0.0) : m_LastValues.at(m_AuthoritativeSourceIndex);
    }

    SignedNormalizedFraction AxisMultiplexor::GetValue() const
    {
        return m_Value;
    }

    void AxisMultiplexor::Tick(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)
    {
        if (m_Sources.empty())
        {
            m_Value = SignedNormalizedFraction(0.0);
            return;
        }

        for (std::size_t index = 0; index < m_Sources.size(); ++index)
        {
            const auto currentValue = ReadValue(m_Sources.at(index));
            if (static_cast<double>(currentValue) != static_cast<double>(m_LastValues.at(index)))
            {
                m_LastValues.at(index) = currentValue;
                m_AuthoritativeSourceIndex = index;
            }
        }

        m_Value = m_LastValues.at(m_AuthoritativeSourceIndex);
    }

    SignedNormalizedFraction AxisMultiplexor::ReadValue(const ::PiSubmarine::Input::Api::IAxis* source)
    {
        return source == nullptr ? SignedNormalizedFraction(0.0) : source->GetValue();
    }
}
