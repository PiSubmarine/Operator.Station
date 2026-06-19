#include "PiSubmarine/Operator/Station/Telemetry/VideoStatusController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    VideoStatusController::VideoStatusController(::PiSubmarine::Video::Telemetry::Api::IProvider& provider, QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
    {
    }

    void VideoStatusController::Refresh()
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
            emit SnapshotChanged(
                m_LastStatus.IsStreamingEnabled,
                m_LastStatus.Subscribers,
                ToQString(m_LastStatus.Operational),
                m_LastStatus.HasAnyFault());
        }
    }

    QString VideoStatusController::ToQString(const ::PiSubmarine::Video::Telemetry::Api::OperationalState state)
    {
        switch (state)
        {
        case ::PiSubmarine::Video::Telemetry::Api::OperationalState::Stopped:
            return "Stopped";
        case ::PiSubmarine::Video::Telemetry::Api::OperationalState::Streaming:
            return "Streaming";
        case ::PiSubmarine::Video::Telemetry::Api::OperationalState::Faulted:
            return "Faulted";
        }

        return "Unknown";
    }
}
