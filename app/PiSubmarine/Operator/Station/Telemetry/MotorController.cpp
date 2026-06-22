#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"

#include "PiSubmarine/Motor/Telemetry/Api/Faults.h"
#include "PiSubmarine/Motor/Telemetry/Api/Warnings.h"

#include <type_traits>

namespace PiSubmarine::Operator::Station::Telemetry
{
    namespace
    {
        template<typename TEnum>
        [[nodiscard]] bool HasFlag(const TEnum value, const TEnum flag)
        {
            using Underlying = std::underlying_type_t<TEnum>;
            return (static_cast<Underlying>(value) & static_cast<Underlying>(flag)) != 0;
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
                static_cast<double>(m_LastState.DriveEffort) * 100.0,
                HasFlag(m_LastState.ActiveFaults, ::PiSubmarine::Motor::Telemetry::Api::Faults::Overcurrent),
                HasFlag(m_LastState.ActiveFaults, ::PiSubmarine::Motor::Telemetry::Api::Faults::Overtemperature),
                HasFlag(m_LastState.ActiveWarnings, ::PiSubmarine::Motor::Telemetry::Api::Warnings::Temperature),
                HasFlag(m_LastState.ActiveFaults, ::PiSubmarine::Motor::Telemetry::Api::Faults::Undervoltage),
                HasFlag(m_LastState.ActiveFaults, ::PiSubmarine::Motor::Telemetry::Api::Faults::Overvoltage),
                HasFlag(m_LastState.ActiveFaults, ::PiSubmarine::Motor::Telemetry::Api::Faults::OpenLoad));
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
