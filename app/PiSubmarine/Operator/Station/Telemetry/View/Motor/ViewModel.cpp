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
    QString ViewModel::GetDirection() const { return m_Direction; }
    double ViewModel::GetDriveEffortPercent() const { return m_DriveEffortPercent; }

    void ViewModel::SetSnapshot(
        const QString& operationalState,
        const bool hasFault,
        const bool hasWarning,
        const QString& direction,
        const double driveEffortPercent)
    {
        if (m_OperationalState == operationalState
            && m_HasFault == hasFault
            && m_HasWarning == hasWarning
            && m_Direction == direction
            && m_DriveEffortPercent == driveEffortPercent)
        {
            return;
        }

        m_OperationalState = operationalState;
        m_HasFault = hasFault;
        m_HasWarning = hasWarning;
        m_Direction = direction;
        m_DriveEffortPercent = driveEffortPercent;
        emit SnapshotChanged();
    }
}
