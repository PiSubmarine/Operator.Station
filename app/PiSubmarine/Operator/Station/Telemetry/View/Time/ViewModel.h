#pragma once

#include <QObject>
#include <QString>

#include "PiSubmarine/Operator/Station/Composition/LeaseState.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Time/FaultState.h"

namespace PiSubmarine::Operator::Station::Telemetry::View::Time
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool hasLease READ GetHasLease NOTIFY SnapshotChanged)
        Q_PROPERTY(QString displayText READ GetDisplayText NOTIFY SnapshotChanged)
        Q_PROPERTY(int faultState READ GetFaultState NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetHasLease() const;
        [[nodiscard]] QString GetDisplayText() const;
        [[nodiscard]] int GetFaultState() const;

    public slots:
        void LeaseStateChanged(const ::PiSubmarine::Operator::Station::Composition::OptionalLeaseId& leaseId);
        void SetSnapshot(const QString& displayText, ::PiSubmarine::Operator::Station::Telemetry::View::Time::FaultState faultState);

    signals:
        void SnapshotChanged();

    private:
        bool m_HasLease = false;
        QString m_DisplayText{"NO LEASE"};
        FaultState m_FaultState = FaultState::Normal;
    };
}
