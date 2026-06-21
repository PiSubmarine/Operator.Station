#pragma once

#include <QObject>

#include "PiSubmarine/Depth/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class DepthController final : public QObject
    {
        Q_OBJECT

    public:
        explicit DepthController(::PiSubmarine::Depth::Telemetry::Api::IProvider& provider, QObject* parent = nullptr);

        void Refresh();

    signals:
        void SnapshotChanged(bool hasDepth, double depthMeters);

    private:
        ::PiSubmarine::Depth::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Depth::Telemetry::Api::State m_LastState{};
        bool m_HasSnapshot = false;
    };
}
