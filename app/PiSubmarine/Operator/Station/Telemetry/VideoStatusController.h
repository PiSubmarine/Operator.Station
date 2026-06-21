#pragma once

#include <QObject>
#include <QString>

#include "PiSubmarine/Video/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class VideoStatusController final : public QObject
    {
        Q_OBJECT

    public:
        explicit VideoStatusController(::PiSubmarine::Video::Telemetry::Api::IProvider& provider, QObject* parent = nullptr);

        void Refresh();

    signals:
        void SnapshotChanged(bool isStreamingEnabled, int subscribers, const QString& operationalState, bool hasFault, const QString& faultSummary);

    private:
        [[nodiscard]] static QString ToQString(::PiSubmarine::Video::Telemetry::Api::OperationalState state);
        [[nodiscard]] static QString ToFaultSummary(::PiSubmarine::Video::Telemetry::Api::Faults faults);

        ::PiSubmarine::Video::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Video::Telemetry::Api::Status m_LastStatus{};
        bool m_HasSnapshot = false;
    };
}
