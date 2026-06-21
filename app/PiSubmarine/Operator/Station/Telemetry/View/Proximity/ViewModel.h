#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Telemetry::View::Proximity
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool hasDistance READ GetHasDistance NOTIFY SnapshotChanged)
        Q_PROPERTY(double distanceMeters READ GetDistanceMeters NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetHasDistance() const;
        [[nodiscard]] double GetDistanceMeters() const;

    public slots:
        void SetSnapshot(bool hasDistance, double distanceMeters);

    signals:
        void SnapshotChanged();

    private:
        bool m_HasDistance = false;
        double m_DistanceMeters = 0.0;
    };
}
