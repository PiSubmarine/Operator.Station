#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Telemetry::View::Lamp
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double intensity READ Intensity NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOvercurrentFault READ HasOvercurrentFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOvertemperatureFault READ HasOvertemperatureFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasTemperatureWarning READ HasTemperatureWarning NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasUndervoltageFault READ HasUndervoltageFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOvervoltageFault READ HasOvervoltageFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasOpenLoadFault READ HasOpenLoadFault NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double Intensity() const;
        [[nodiscard]] bool HasOvercurrentFault() const;
        [[nodiscard]] bool HasOvertemperatureFault() const;
        [[nodiscard]] bool HasTemperatureWarning() const;
        [[nodiscard]] bool HasUndervoltageFault() const;
        [[nodiscard]] bool HasOvervoltageFault() const;
        [[nodiscard]] bool HasOpenLoadFault() const;

    public slots:
        void SetSnapshot(
            double intensity,
            bool hasOvercurrentFault,
            bool hasOvertemperatureFault,
            bool hasTemperatureWarning,
            bool hasUndervoltageFault,
            bool hasOvervoltageFault,
            bool hasOpenLoadFault);

    signals:
        void SnapshotChanged();

    private:
        double m_Intensity = 0.0;
        bool m_HasOvercurrentFault = false;
        bool m_HasOvertemperatureFault = false;
        bool m_HasTemperatureWarning = false;
        bool m_HasUndervoltageFault = false;
        bool m_HasOvervoltageFault = false;
        bool m_HasOpenLoadFault = false;
    };
}
