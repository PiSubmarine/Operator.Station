#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Telemetry::View::Video
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool isStreamingEnabled READ GetIsStreamingEnabled NOTIFY SnapshotChanged)
        Q_PROPERTY(int subscribers READ GetSubscribers NOTIFY SnapshotChanged)
        Q_PROPERTY(QString operationalState READ GetOperationalState NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasFault READ GetHasFault NOTIFY SnapshotChanged)
        Q_PROPERTY(bool hasSnapshot READ GetHasSnapshot NOTIFY SnapshotChanged)
        Q_PROPERTY(bool isOverlayVisible READ GetIsOverlayVisible NOTIFY SnapshotChanged)
        Q_PROPERTY(QString overlayMessage READ GetOverlayMessage NOTIFY SnapshotChanged)
        Q_PROPERTY(QString overlayBackgroundColor READ GetOverlayBackgroundColor NOTIFY SnapshotChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetIsStreamingEnabled() const;
        [[nodiscard]] int GetSubscribers() const;
        [[nodiscard]] QString GetOperationalState() const;
        [[nodiscard]] bool GetHasFault() const;
        [[nodiscard]] bool GetHasSnapshot() const;
        [[nodiscard]] bool GetIsOverlayVisible() const;
        [[nodiscard]] QString GetOverlayMessage() const;
        [[nodiscard]] QString GetOverlayBackgroundColor() const;

    public slots:
        void SetSnapshot(
            bool isStreamingEnabled,
            int subscribers,
            const QString& operationalState,
            bool hasFault,
            const QString& faultSummary);

    signals:
        // TODO Is it used anywhere? If not, remove.
        void SnapshotChanged();

    private:
        bool m_IsStreamingEnabled = false;
        int m_Subscribers = 0;
        QString m_OperationalState{"Stopped"};
        bool m_HasFault = false;
        bool m_HasSnapshot = false;
        QString m_FaultSummary;
    };
}
