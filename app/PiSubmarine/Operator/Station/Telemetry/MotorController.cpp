#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"

#include <type_traits>

namespace PiSubmarine::Operator::Station::Telemetry
{
    namespace
    {
        [[nodiscard]] bool HasAnyBits(const auto value)
        {
            using Underlying = std::underlying_type_t<decltype(value)>;
            return static_cast<Underlying>(value) != 0;
        }
    }

    MotorController::MotorController(::PiSubmarine::Motor::Telemetry::Api::IProvider& provider, QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
    {
    }

    void MotorController::Refresh()
    {
        const auto stateResult = m_Provider.GetState();
        if (!stateResult.has_value())
        {
            // TODO User-visible error reporting.
            return;
        }

        if (!m_HasSnapshot || m_LastState != *stateResult)
        {
            m_LastState = *stateResult;
            m_HasSnapshot = true;
            emit SnapshotChanged(
                ToQString(m_LastState.Operational),
                HasAnyBits(m_LastState.ActiveFaults),
                HasAnyBits(m_LastState.ActiveWarnings));
        }
    }

    QString MotorController::ToQString(const ::PiSubmarine::Motor::Telemetry::Api::OperationalState state)
    {
        switch (state)
        {
        case ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Operational:
            return "Operational";
        case ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Degraded:
            return "Degraded";
        case ::PiSubmarine::Motor::Telemetry::Api::OperationalState::Faulted:
            return "Faulted";
        }

        return "Unknown";
    }
}
