#include "PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Lamp
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    double ViewModel::Intensity() const { return m_Intensity; }
    bool ViewModel::HasFault() const { return m_HasFault; }
    bool ViewModel::HasWarning() const { return m_HasWarning; }

    void ViewModel::SetSnapshot(const double intensity, const bool hasFault, const bool hasWarning)
    {
        if (m_Intensity == intensity && m_HasFault == hasFault && m_HasWarning == hasWarning)
        {
            return;
        }

        m_Intensity = intensity;
        m_HasFault = hasFault;
        m_HasWarning = hasWarning;
        emit SnapshotChanged();
    }
}
