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

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] bool GetIsStreamingEnabled() const;
        [[nodiscard]] int GetSubscribers() const;
        [[nodiscard]] QString GetOperationalState() const;
        [[nodiscard]] bool GetHasFault() const;

    public slots:
        void SetSnapshot(bool isStreamingEnabled, int subscribers, const QString& operationalState, bool hasFault);

    signals:
        void SnapshotChanged();

    private:
        bool m_IsStreamingEnabled = false;
        int m_Subscribers = 0;
        QString m_OperationalState{"Stopped"};
        bool m_HasFault = false;
    };
}
