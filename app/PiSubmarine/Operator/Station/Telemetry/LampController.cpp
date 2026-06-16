#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    LampController::LampController(::PiSubmarine::Lamp::Telemetry::Api::IProvider& provider, QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
    {
    }

    void LampController::Refresh()
    {
        const auto statusResult = m_Provider.GetStatus();
        if (!statusResult.has_value())
        {
            return;
        }

        if (!m_HasSnapshot || m_LastStatus != *statusResult)
        {
            m_LastStatus = *statusResult;
            m_HasSnapshot = true;
            emit SnapshotChanged(m_LastStatus.IsActive, m_LastStatus.HasAnyFault(), m_LastStatus.HasAnyWarning());
        }
    }
}
