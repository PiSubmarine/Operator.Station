#include "PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Lamp
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    double ViewModel::Intensity() const { return m_Intensity; }
    bool ViewModel::HasOvercurrentFault() const { return m_HasOvercurrentFault; }
    bool ViewModel::HasOvertemperatureFault() const { return m_HasOvertemperatureFault; }
    bool ViewModel::HasTemperatureWarning() const { return m_HasTemperatureWarning; }
    bool ViewModel::HasUndervoltageFault() const { return m_HasUndervoltageFault; }
    bool ViewModel::HasOvervoltageFault() const { return m_HasOvervoltageFault; }
    bool ViewModel::HasOpenLoadFault() const { return m_HasOpenLoadFault; }

    void ViewModel::SetSnapshot(
        const double intensity,
        const bool hasOvercurrentFault,
        const bool hasOvertemperatureFault,
        const bool hasTemperatureWarning,
        const bool hasUndervoltageFault,
        const bool hasOvervoltageFault,
        const bool hasOpenLoadFault)
    {
        if (m_Intensity == intensity &&
            m_HasOvercurrentFault == hasOvercurrentFault &&
            m_HasOvertemperatureFault == hasOvertemperatureFault &&
            m_HasTemperatureWarning == hasTemperatureWarning &&
            m_HasUndervoltageFault == hasUndervoltageFault &&
            m_HasOvervoltageFault == hasOvervoltageFault &&
            m_HasOpenLoadFault == hasOpenLoadFault)
        {
            return;
        }

        m_Intensity = intensity;
        m_HasOvercurrentFault = hasOvercurrentFault;
        m_HasOvertemperatureFault = hasOvertemperatureFault;
        m_HasTemperatureWarning = hasTemperatureWarning;
        m_HasUndervoltageFault = hasUndervoltageFault;
        m_HasOvervoltageFault = hasOvervoltageFault;
        m_HasOpenLoadFault = hasOpenLoadFault;
        emit SnapshotChanged();
    }
}
