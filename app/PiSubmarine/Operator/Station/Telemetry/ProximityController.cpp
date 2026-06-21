#include "PiSubmarine/Operator/Station/Telemetry/ProximityController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    ProximityController::ProximityController(::PiSubmarine::Proximity::Telemetry::Api::IProvider& provider, QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
    {
    }

    void ProximityController::Refresh()
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
                m_LastState.Distance.has_value(),
                m_LastState.Distance.has_value() ? m_LastState.Distance->Value : 0.0);
        }
    }
}
