#pragma once

#include <QObject>

#include "PiSubmarine/Lamp/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class LampController final : public QObject
    {
        Q_OBJECT

    public:
        explicit LampController(::PiSubmarine::Lamp::Telemetry::Api::IProvider& provider, QObject* parent = nullptr);

        void Refresh();

    signals:
        void SnapshotChanged(double intensity, bool hasFault, bool hasWarning);

    private:
        ::PiSubmarine::Lamp::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Lamp::Telemetry::Api::Status m_LastStatus{};
        bool m_HasSnapshot = false;
    };
}
