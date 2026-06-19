#include "PiSubmarine/Operator/Station/Telemetry/BallastController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    BallastController::BallastController(::PiSubmarine::Ballast::Telemetry::Api::IProvider& provider, QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
    {
    }

    void BallastController::Refresh()
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
                m_LastState.Position.has_value(),
                m_LastState.Position.has_value() ? static_cast<double>(*m_LastState.Position) : 0.0);
        }
    }
}
