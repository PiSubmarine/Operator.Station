#pragma once

#include <QObject>

#include "PiSubmarine/Ballast/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class BallastController final : public QObject
    {
        Q_OBJECT

    public:
        explicit BallastController(::PiSubmarine::Ballast::Telemetry::Api::IProvider& provider, QObject* parent = nullptr);

        void Refresh();

    signals:
        void SnapshotChanged(bool hasPosition, double position);

    private:
        ::PiSubmarine::Ballast::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Ballast::Telemetry::Api::State m_LastState{};
        bool m_HasSnapshot = false;
    };
}
