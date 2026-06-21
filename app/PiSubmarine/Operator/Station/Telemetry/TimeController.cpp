#include "PiSubmarine/Operator/Station/Telemetry/TimeController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    namespace
    {
        constexpr auto WarningThreshold = std::chrono::milliseconds(100);
        constexpr auto ErrorThreshold = std::chrono::seconds(1);
    }

    TimeController::TimeController(
        ::PiSubmarine::Time::Telemetry::Api::IProvider& provider,
        std::function<bool()> hasLease,
        QObject* parent)
        : QObject(parent)
        , m_Provider(provider)
        , m_HasLease(std::move(hasLease))
    {
    }

    void TimeController::Refresh(const std::chrono::nanoseconds& controllerUptime)
    {
        const bool hasLease = m_HasLease();
        if (!hasLease)
        {
            Publish(false, "NO LEASE", "#8f1d1d");
            m_LastLeaseState = false;
            return;
        }

        if (!m_LastLeaseState)
        {
            m_LastTelemetryChangeAt = controllerUptime;
        }
        m_LastLeaseState = true;

        const auto stateResult = m_Provider.GetState();
        if (stateResult.has_value())
        {
            if (!m_LastTelemetryUptime.has_value() || *m_LastTelemetryUptime != stateResult->Uptime)
            {
                m_LastTelemetryUptime = stateResult->Uptime;
                m_LastTelemetryChangeAt = controllerUptime;
            }
        }

        const auto displayText = FormatUptime(m_LastTelemetryUptime.value_or(std::chrono::nanoseconds::zero()));
        const auto ageSinceChange = controllerUptime - m_LastTelemetryChangeAt;
        Publish(true, displayText, SelectBackgroundColor(ageSinceChange));
    }

    QString TimeController::FormatUptime(const std::chrono::nanoseconds uptime)
    {
        const auto totalSeconds = std::chrono::duration_cast<std::chrono::seconds>(uptime).count();
        const auto hours = totalSeconds / 3600;
        const auto minutes = (totalSeconds / 60) % 60;
        const auto seconds = totalSeconds % 60;

        return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }

    QString TimeController::SelectBackgroundColor(const std::chrono::nanoseconds ageSinceChange)
    {
        if (ageSinceChange > ErrorThreshold)
        {
            return "#8f1d1d";
        }

        if (ageSinceChange > WarningThreshold)
        {
            return "#b78a1e";
        }

        return "#123247";
    }

    void TimeController::Publish(const bool hasLease, const QString& displayText, const QString& backgroundColor)
    {
        if (m_HasSnapshot &&
            m_LastLeaseState == hasLease &&
            m_LastDisplayText == displayText &&
            m_LastBackgroundColor == backgroundColor)
        {
            return;
        }

        m_HasSnapshot = true;
        m_LastLeaseState = hasLease;
        m_LastDisplayText = displayText;
        m_LastBackgroundColor = backgroundColor;
        emit SnapshotChanged(hasLease, displayText, backgroundColor);
    }
}
