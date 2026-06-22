#include "PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Motor
{
    ViewModel::ViewModel(const QString& panelLabel, const bool fillFromTop, QObject* parent)
        : QObject(parent)
        , m_PanelLabel(panelLabel)
        , m_FillFromTop(fillFromTop)
    {
    }

    QString ViewModel::GetPanelLabel() const { return m_PanelLabel; }
    bool ViewModel::GetFillFromTop() const { return m_FillFromTop; }
    QString ViewModel::GetOperationalState() const { return m_OperationalState; }
    double ViewModel::GetDriveEffortPercent() const { return m_DriveEffortPercent; }
    QString ViewModel::GetPrimaryColor() const
    {
        if (m_OperationalState == "Faulted")
        {
            return "#d64545";
        }

        if (m_OperationalState == "Degraded")
        {
            return "#e0b53a";
        }

        return "#3b82b6";
    }
    bool ViewModel::HasOvercurrentFault() const { return m_HasOvercurrentFault; }
    bool ViewModel::HasOvertemperatureFault() const { return m_HasOvertemperatureFault; }
    bool ViewModel::HasTemperatureWarning() const { return m_HasTemperatureWarning; }
    bool ViewModel::HasUndervoltageFault() const { return m_HasUndervoltageFault; }
    bool ViewModel::HasOvervoltageFault() const { return m_HasOvervoltageFault; }
    bool ViewModel::HasOpenLoadFault() const { return m_HasOpenLoadFault; }

    void ViewModel::SetSnapshot(
        const QString& operationalState,
        const double driveEffortPercent,
        const bool hasOvercurrentFault,
        const bool hasOvertemperatureFault,
        const bool hasTemperatureWarning,
        const bool hasUndervoltageFault,
        const bool hasOvervoltageFault,
        const bool hasOpenLoadFault)
    {
        if (m_OperationalState == operationalState
            && m_DriveEffortPercent == driveEffortPercent
            && m_HasOvercurrentFault == hasOvercurrentFault
            && m_HasOvertemperatureFault == hasOvertemperatureFault
            && m_HasTemperatureWarning == hasTemperatureWarning
            && m_HasUndervoltageFault == hasUndervoltageFault
            && m_HasOvervoltageFault == hasOvervoltageFault
            && m_HasOpenLoadFault == hasOpenLoadFault)
            {
                return;
            }

        m_OperationalState = operationalState;
        m_DriveEffortPercent = driveEffortPercent;
        m_HasOvercurrentFault = hasOvercurrentFault;
        m_HasOvertemperatureFault = hasOvertemperatureFault;
        m_HasTemperatureWarning = hasTemperatureWarning;
        m_HasUndervoltageFault = hasUndervoltageFault;
        m_HasOvervoltageFault = hasOvervoltageFault;
        m_HasOpenLoadFault = hasOpenLoadFault;
        emit SnapshotChanged();
    }
}
