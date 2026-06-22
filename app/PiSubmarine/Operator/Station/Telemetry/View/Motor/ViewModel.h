#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Telemetry::View::Motor
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString panelLabel READ GetPanelLabel CONSTANT)
        Q_PROPERTY(bool fillFromTop READ GetFillFromTop CONSTANT)
        Q_PROPERTY(QString operationalState READ GetOperationalState NOTIFY SnapshotChanged)
        Q_PROPERTY(double driveEffortPercent READ GetDriveEffortPercent NOTIFY SnapshotChanged)
        Q_PROPERTY(QString primaryColor READ GetPrimaryColor NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOvercurrentFault READ HasOvercurrentFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOvertemperatureFault READ HasOvertemperatureFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasTemperatureWarning READ HasTemperatureWarning NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasUndervoltageFault READ HasUndervoltageFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOvervoltageFault READ HasOvervoltageFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOpenLoadFault READ HasOpenLoadFault NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(const QString& panelLabel, bool fillFromTop, QObject* parent = nullptr);

        [[nodiscard]] QString GetPanelLabel() const;
        [[nodiscard]] bool GetFillFromTop() const;
        [[nodiscard]] QString GetOperationalState() const;
        [[nodiscard]] double GetDriveEffortPercent() const;
        [[nodiscard]] QString GetPrimaryColor() const;
        [[nodiscard]] bool HasOvercurrentFault() const;
        [[nodiscard]] bool HasOvertemperatureFault() const;
        [[nodiscard]] bool HasTemperatureWarning() const;
        [[nodiscard]] bool HasUndervoltageFault() const;
        [[nodiscard]] bool HasOvervoltageFault() const;
        [[nodiscard]] bool HasOpenLoadFault() const;

    public slots:
        void SetSnapshot(
            const QString& operationalState,
            double driveEffortPercent,
            bool hasOvercurrentFault,
            bool hasOvertemperatureFault,
            bool hasTemperatureWarning,
            bool hasUndervoltageFault,
            bool hasOvervoltageFault,
            bool hasOpenLoadFault);

    signals:
        void SnapshotChanged();

    private:
        QString m_PanelLabel;
        bool m_FillFromTop = false;
        QString m_OperationalState{"Operational"};
        double m_DriveEffortPercent = 0.0;
        bool m_HasOvercurrentFault = false;
        bool m_HasOvertemperatureFault = false;
        bool m_HasTemperatureWarning = false;
        bool m_HasUndervoltageFault = false;
        bool m_HasOvervoltageFault = false;
        bool m_HasOpenLoadFault = false;
    };
}
