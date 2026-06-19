#pragma once

#include <QObject>

#include "PiSubmarine/Proximity/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class ProximityController final : public QObject
    {
        Q_OBJECT

    public:
        explicit ProximityController(::PiSubmarine::Proximity::Telemetry::Api::IProvider& provider, QObject* parent = nullptr);

        void Refresh();

    signals:
        void SnapshotChanged(bool hasDistance, double distanceMeters);

    private:
        ::PiSubmarine::Proximity::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Proximity::Telemetry::Api::State m_LastState{};
        bool m_HasSnapshot = false;
    };
}
