#pragma once

#include <QObject>

#include "PiSubmarine/Battery/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class BatteryController final : public QObject
    {
        Q_OBJECT

    public:
        explicit BatteryController(::PiSubmarine::Battery::Telemetry::Api::IProvider& provider, QObject* parent = nullptr);

        void Refresh();

    signals:
        void SnapshotChanged(double packVoltage, double packCurrent, double stateOfCharge, double packTemperature);

    private:
        ::PiSubmarine::Battery::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Battery::Telemetry::Api::State m_LastState{};
        bool m_HasSnapshot = false;
    };
}
