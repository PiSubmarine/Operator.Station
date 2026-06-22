#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Telemetry::View::Motor
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString operationalState READ GetOperationalState NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasFault READ HasFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasWarning READ HasWarning NOTIFY SnapshotChanged)
        Q_PROPERTY(QString direction READ GetDirection NOTIFY SnapshotChanged)
        Q_PROPERTY(double driveEffortPercent READ GetDriveEffortPercent NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] QString GetOperationalState() const;
        [[nodiscard]] bool HasFault() const;
        [[nodiscard]] bool HasWarning() const;
        [[nodiscard]] QString GetDirection() const;
        [[nodiscard]] double GetDriveEffortPercent() const;

    public slots:
        void SetSnapshot(
            const QString& operationalState,
            bool hasFault,
            bool hasWarning,
            const QString& direction,
            double driveEffortPercent);

    signals:
        void SnapshotChanged();

    private:
        QString m_OperationalState{"Operational"};
        bool m_HasFault = false;
        bool m_HasWarning = false;
        QString m_Direction{"Idle"};
        double m_DriveEffortPercent = 0.0;
    };
}
