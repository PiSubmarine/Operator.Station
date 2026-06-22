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
            // TODO User-visible error reporting.
            return;
        }

        if (!m_HasSnapshot || m_LastState != *stateResult)
        {
            m_LastState = *stateResult;
            m_HasSnapshot = true;
            emit SnapshotChanged(
                m_LastState.PackVoltage.Value,
                m_LastState.PackCurrent.Value,
                m_LastState.PackTemperature.Value,
                m_LastState.ChargerVoltage.Value,
                m_LastState.ChargerCurrent.Value,
                m_LastState.ChargerTemperature.Value,
                static_cast<double>(m_LastState.StateOfCharge),
                m_LastState.TimeToFull.has_value(),
                m_LastState.TimeToFull.has_value() ? m_LastState.TimeToFull->count() : 0,
                m_LastState.TimeToEmpty.has_value(),
                m_LastState.TimeToEmpty.has_value() ? m_LastState.TimeToEmpty->count() : 0);
        }
    }
}
