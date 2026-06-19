#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Telemetry::View::Battery
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double packVoltage READ GetPackVoltage NOTIFY SnapshotChanged)
        Q_PROPERTY(double packCurrent READ GetPackCurrent NOTIFY SnapshotChanged)
        Q_PROPERTY(double stateOfCharge READ GetStateOfCharge NOTIFY SnapshotChanged)
        Q_PROPERTY(double packTemperature READ GetPackTemperature NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double GetPackVoltage() const;
        [[nodiscard]] double GetPackCurrent() const;
        [[nodiscard]] double GetStateOfCharge() const;
        [[nodiscard]] double GetPackTemperature() const;

    public slots:
        void SetSnapshot(double packVoltage, double packCurrent, double stateOfCharge, double packTemperature);

    signals:
        void SnapshotChanged();

    private:
        double m_PackVoltage = 0.0;
        double m_PackCurrent = 0.0;
        double m_StateOfCharge = 0.0;
        double m_PackTemperature = 0.0;
    };
}
