#pragma once

#include <chrono>
#include <functional>
#include <optional>

#include <QObject>
#include <QString>

#include "PiSubmarine/Time/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class TimeController final : public QObject
    {
        Q_OBJECT

    public:
        explicit TimeController(
            ::PiSubmarine::Time::Telemetry::Api::IProvider& provider,
            std::function<bool()> hasLease,
            QObject* parent = nullptr);

        void Refresh(const std::chrono::nanoseconds& controllerUptime);

    signals:
        void SnapshotChanged(bool hasLease, const QString& displayText, const QString& backgroundColor);

    private:
        [[nodiscard]] static QString FormatUptime(std::chrono::nanoseconds uptime);
        [[nodiscard]] static QString SelectBackgroundColor(std::chrono::nanoseconds ageSinceChange);
        void Publish(bool hasLease, const QString& displayText, const QString& backgroundColor);

        ::PiSubmarine::Time::Telemetry::Api::IProvider& m_Provider;
        std::function<bool()> m_HasLease;
        std::optional<std::chrono::nanoseconds> m_LastTelemetryUptime;
        std::chrono::nanoseconds m_LastTelemetryChangeAt{0};
        bool m_LastLeaseState = false;
        QString m_LastDisplayText{"00:00:00"};
        QString m_LastBackgroundColor{"#123247"};
        bool m_HasSnapshot = false;
    };
}
