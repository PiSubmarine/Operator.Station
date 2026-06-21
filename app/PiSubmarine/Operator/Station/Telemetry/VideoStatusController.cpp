#include "PiSubmarine/Operator/Station/Telemetry/VideoStatusController.h"

#include <cstdint>

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
            // TODO User-visible error reporting.
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
                m_LastStatus.HasAnyFault(),
                ToFaultSummary(m_LastStatus.ActiveFaults));
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

    QString VideoStatusController::ToFaultSummary(const ::PiSubmarine::Video::Telemetry::Api::Faults faults)
    {
        const auto rawFaults = static_cast<std::uint32_t>(faults);
        const auto sourceError = static_cast<std::uint32_t>(::PiSubmarine::Video::Telemetry::Api::Faults::SourceError);
        const auto configError = static_cast<std::uint32_t>(::PiSubmarine::Video::Telemetry::Api::Faults::ConfigError);
        const auto networkError = static_cast<std::uint32_t>(::PiSubmarine::Video::Telemetry::Api::Faults::NetworkError);

        if ((rawFaults & (sourceError | configError)) != 0)
        {
            return "CAMERA ERROR";
        }

        if ((rawFaults & networkError) != 0)
        {
            return "NETWORK ERROR";
        }

        return "CAMERA ERROR";
    }
}
