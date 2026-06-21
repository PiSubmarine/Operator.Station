#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Telemetry::View::Ballast
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool hasPosition READ GetHasPosition NOTIFY SnapshotChanged)
        Q_PROPERTY(double position READ GetPosition NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetHasPosition() const;
        [[nodiscard]] double GetPosition() const;

    public slots:
        void SetSnapshot(bool hasPosition, double position);

    signals:
        void SnapshotChanged();

    private:
        bool m_HasPosition = false;
        double m_Position = 0.0;
    };
}
