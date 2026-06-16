#pragma once

#include <QObject>

#include "PiSubmarine/Motor/Telemetry/Api/IProvider.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    class MotorController final : public QObject
    {
        Q_OBJECT

    public:
        explicit MotorController(::PiSubmarine::Motor::Telemetry::Api::IProvider& provider, QObject* parent = nullptr);

        void Refresh();

    signals:
        void SnapshotChanged(const QString& operationalState, bool hasFault, bool hasWarning);

    private:
        [[nodiscard]] static QString ToQString(::PiSubmarine::Motor::Telemetry::Api::OperationalState state);

        ::PiSubmarine::Motor::Telemetry::Api::IProvider& m_Provider;
        ::PiSubmarine::Motor::Telemetry::Api::State m_LastState{};
        bool m_HasSnapshot = false;
    };
}
