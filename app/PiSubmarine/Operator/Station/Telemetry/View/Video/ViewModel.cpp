#include "PiSubmarine/Operator/Station/Telemetry/View/Video/ViewModel.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Video
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    bool ViewModel::GetIsStreamingEnabled() const { return m_IsStreamingEnabled; }
    int ViewModel::GetSubscribers() const { return m_Subscribers; }
    QString ViewModel::GetOperationalState() const { return m_OperationalState; }
    bool ViewModel::GetHasFault() const { return m_HasFault; }

    void ViewModel::SetSnapshot(
        const bool isStreamingEnabled,
        const int subscribers,
        const QString& operationalState,
        const bool hasFault)
    {
        if (m_IsStreamingEnabled == isStreamingEnabled &&
            m_Subscribers == subscribers &&
            m_OperationalState == operationalState &&
            m_HasFault == hasFault)
        {
            return;
        }

        m_IsStreamingEnabled = isStreamingEnabled;
        m_Subscribers = subscribers;
        m_OperationalState = operationalState;
        m_HasFault = hasFault;
        emit SnapshotChanged();
    }
}
