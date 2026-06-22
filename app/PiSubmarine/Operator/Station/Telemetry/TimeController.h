#pragma once

#include <chrono>
#include <optional>

#include <QObject>
#include <QString>

#include "PiSubmarine/Operator/Station/Composition/LeaseState.h"
#include "PiSubmarine/Time/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class TimeController final : public QObject
    {
        Q_OBJECT

    public:
        explicit TimeController(
            ::PiSubmarine::Time::Telemetry::Api::IProvider& provider,
            QObject* parent = nullptr);

        void Refresh(const std::chrono::nanoseconds& controllerUptime);

    public slots:
        void LeaseStateChanged(const ::PiSubmarine::Operator::Station::Composition::OptionalLeaseId& leaseId);

    signals:
        void SnapshotChanged(const QString& displayText, const QString& backgroundColor);

    private:
        [[nodiscard]] static QString FormatUptime(std::chrono::nanoseconds uptime);
        [[nodiscard]] static QString SelectBackgroundColor(std::chrono::nanoseconds ageSinceChange);
        void Publish(const QString& displayText, const QString& backgroundColor);

        ::PiSubmarine::Time::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Operator::Station::Composition::OptionalLeaseId m_LeaseId;
        std::optional<std::chrono::nanoseconds> m_LastTelemetryUptime;
        std::chrono::nanoseconds m_LastTelemetryChangeAt{0};
        QString m_LastDisplayText{"00:00:00"};
        QString m_LastBackgroundColor{"#123247"};
        bool m_HasSnapshot = false;
        bool m_HadLease = false;
    };
}
