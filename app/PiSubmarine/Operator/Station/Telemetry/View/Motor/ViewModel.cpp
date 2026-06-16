#include "PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Motor
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    QString ViewModel::GetOperationalState() const { return m_OperationalState; }
    bool ViewModel::HasFault() const { return m_HasFault; }
    bool ViewModel::HasWarning() const { return m_HasWarning; }

    void ViewModel::SetSnapshot(const QString& operationalState, const bool hasFault, const bool hasWarning)
    {
        if (m_OperationalState == operationalState && m_HasFault == hasFault && m_HasWarning == hasWarning)
        {
            return;
        }

        m_OperationalState = operationalState;
        m_HasFault = hasFault;
        m_HasWarning = hasWarning;
        emit SnapshotChanged();
    }
}
