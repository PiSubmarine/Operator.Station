#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Telemetry::View::Lamp
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double intensity READ Intensity NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasFault READ HasFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasWarning READ HasWarning NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] double Intensity() const;
        [[nodiscard]] bool HasFault() const;
        [[nodiscard]] bool HasWarning() const;

    public slots:
        void SetSnapshot(double intensity, bool hasFault, bool hasWarning);

    signals:
        void SnapshotChanged();

    private:
        double m_Intensity = 0.0;
        bool m_HasFault = false;
        bool m_HasWarning = false;
    };
}
