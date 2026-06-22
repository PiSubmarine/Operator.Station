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
    bool ViewModel::GetHasSnapshot() const { return m_HasSnapshot; }
    bool ViewModel::GetIsOverlayVisible() const
    {
        return m_HasSnapshot && (m_HasFault || m_OperationalState == "Stopped");
    }

    QString ViewModel::GetOverlayMessage() const
    {
        if (m_HasFault)
        {
            return m_FaultSummary;
        }

        if (m_OperationalState == "Stopped")
        {
            return "CAMERA OFF";
        }

        return {};
    }

    QString ViewModel::GetOverlayBackgroundColor() const
    {
        if (m_HasFault)
        {
            return "#8f1d1d";
        }

        if (m_OperationalState == "Stopped")
        {
            return "#f0091823";
        }

        return "transparent";
    }

    void ViewModel::SetSnapshot(
        const bool isStreamingEnabled,
        const int subscribers,
        const QString& operationalState,
        const bool hasFault,
        const QString& faultSummary)
    {
        if (m_IsStreamingEnabled == isStreamingEnabled &&
            m_Subscribers == subscribers &&
            m_OperationalState == operationalState &&
            m_HasFault == hasFault &&
            m_FaultSummary == faultSummary)
        {
            return;
        }

        m_IsStreamingEnabled = isStreamingEnabled;
        m_Subscribers = subscribers;
        m_OperationalState = operationalState;
        m_HasFault = hasFault;
        m_HasSnapshot = true;
        m_FaultSummary = faultSummary;
        emit SnapshotChanged();
    }
}
