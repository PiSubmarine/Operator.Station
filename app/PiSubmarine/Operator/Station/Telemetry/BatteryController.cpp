#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    BatteryController::BatteryController(::PiSubmarine::Battery::Telemetry::Api::IProvider& provider, QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
    {
    }

    void BatteryController::Refresh()
    {
        const auto stateResult = m_Provider.GetState();
        if (!stateResult.has_value())
        {
            return;
        }

        if (!m_HasSnapshot || m_LastState != *stateResult)
        {
            m_LastState = *stateResult;
            m_HasSnapshot = true;
            emit SnapshotChanged(
                m_LastState.PackVoltage.Value,
                m_LastState.PackCurrent.Value,
                static_cast<double>(m_LastState.StateOfCharge),
                m_LastState.PackTemperature.Value);
        }
    }
}
