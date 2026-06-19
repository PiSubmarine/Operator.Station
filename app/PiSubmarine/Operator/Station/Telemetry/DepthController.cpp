#include "PiSubmarine/Operator/Station/Telemetry/DepthController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    DepthController::DepthController(::PiSubmarine::Depth::Telemetry::Api::IProvider& provider, QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
    {
    }

    void DepthController::Refresh()
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
                m_LastState.Depth.has_value(),
                m_LastState.Depth.has_value() ? m_LastState.Depth->Value : 0.0);
        }
    }
}
