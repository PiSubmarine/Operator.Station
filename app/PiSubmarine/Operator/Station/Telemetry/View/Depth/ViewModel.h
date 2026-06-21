#pragma once

#include <QObject>

namespace PiSubmarine::Operator::Station::Telemetry::View::Depth
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool hasDepth READ GetHasDepth NOTIFY SnapshotChanged)
        Q_PROPERTY(double depthMeters READ GetDepthMeters NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetHasDepth() const;
        [[nodiscard]] double GetDepthMeters() const;

    public slots:
        void SetSnapshot(bool hasDepth, double depthMeters);

    signals:
        void SnapshotChanged();

    private:
        bool m_HasDepth = false;
        double m_DepthMeters = 0.0;
    };
}
