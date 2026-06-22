#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Telemetry::View::Battery
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double packVoltage READ GetPackVoltage NOTIFY SnapshotChanged)
        Q_PROPERTY(double packCurrent READ GetPackCurrent NOTIFY SnapshotChanged)
        Q_PROPERTY(double packTemperature READ GetPackTemperature NOTIFY SnapshotChanged)
        Q_PROPERTY(double chargerVoltage READ GetChargerVoltage NOTIFY SnapshotChanged)
        Q_PROPERTY(double chargerCurrent READ GetChargerCurrent NOTIFY SnapshotChanged)
        Q_PROPERTY(double chargerTemperature READ GetChargerTemperature NOTIFY SnapshotChanged)
        Q_PROPERTY(double stateOfCharge READ GetStateOfCharge NOTIFY SnapshotChanged)
        Q_PROPERTY(QString timeToFullText READ GetTimeToFullText NOTIFY SnapshotChanged)
        Q_PROPERTY(QString timeToEmptyText READ GetTimeToEmptyText NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double GetPackVoltage() const;
        [[nodiscard]] double GetPackCurrent() const;
        [[nodiscard]] double GetPackTemperature() const;
        [[nodiscard]] double GetChargerVoltage() const;
        [[nodiscard]] double GetChargerCurrent() const;
        [[nodiscard]] double GetChargerTemperature() const;
        [[nodiscard]] double GetStateOfCharge() const;
        [[nodiscard]] QString GetTimeToFullText() const;
        [[nodiscard]] QString GetTimeToEmptyText() const;

    public slots:
        void SetSnapshot(
            double packVoltage,
            double packCurrent,
            double packTemperature,
            double chargerVoltage,
            double chargerCurrent,
            double chargerTemperature,
            double stateOfCharge,
            bool hasTimeToFull,
            qint64 timeToFullMilliseconds,
            bool hasTimeToEmpty,
            qint64 timeToEmptyMilliseconds);

    signals:
        void SnapshotChanged();

    private:
        [[nodiscard]] static QString FormatDuration(bool hasValue, qint64 milliseconds);

        double m_PackVoltage = 0.0;
        double m_PackCurrent = 0.0;
        double m_PackTemperature = 0.0;
        double m_ChargerVoltage = 0.0;
        double m_ChargerCurrent = 0.0;
        double m_ChargerTemperature = 0.0;
        double m_StateOfCharge = 0.0;
        QString m_TimeToFullText = "--:--";
        QString m_TimeToEmptyText = "--:--";
    };
}
