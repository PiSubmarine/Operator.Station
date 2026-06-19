#include "PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Lamp
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool ViewModel::IsActive() const { return m_IsActive; }
    bool ViewModel::HasFault() const { return m_HasFault; }
    bool ViewModel::HasWarning() const { return m_HasWarning; }

    void ViewModel::SetSnapshot(const bool isActive, const bool hasFault, const bool hasWarning)
    {
        if (m_IsActive == isActive && m_HasFault == hasFault && m_HasWarning == hasWarning)
        {
            return;
        }

        m_IsActive = isActive;
        m_HasFault = hasFault;
        m_HasWarning = hasWarning;
        emit SnapshotChanged();
    }
}
