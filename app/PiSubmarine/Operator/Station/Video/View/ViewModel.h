#pragma once

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Video::View
{
    class ViewModel final : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(QString receiveBindAddress READ GetReceiveBindAddress WRITE SetReceiveBindAddress NOTIFY ReceiveEndpointChanged)
        Q_PROPERTY(quint16 receivePort READ GetReceivePort WRITE SetReceivePort NOTIFY ReceiveEndpointChanged)
        Q_PROPERTY(QString subscriptionHost READ GetSubscriptionHost WRITE SetSubscriptionHost NOTIFY SubscriptionEndpointChanged)
        Q_PROPERTY(quint16 subscriptionPort READ GetSubscriptionPort WRITE SetSubscriptionPort NOTIFY SubscriptionEndpointChanged)

    public:
        explicit ViewModel(QObject* parent = nullptr);

        [[nodiscard]] QString GetReceiveBindAddress() const;
        void SetReceiveBindAddress(const QString& receiveBindAddress);

        [[nodiscard]] quint16 GetReceivePort() const;
        void SetReceivePort(quint16 receivePort);

        [[nodiscard]] QString GetSubscriptionHost() const;
        void SetSubscriptionHost(const QString& subscriptionHost);

        [[nodiscard]] quint16 GetSubscriptionPort() const;
        void SetSubscriptionPort(quint16 subscriptionPort);

    signals:
        void ReceiveEndpointChanged(const QString& bindAddress, quint16 port);
        void SubscriptionEndpointChanged(const QString& host, quint16 port);

    private:
        QString m_ReceiveBindAddress{"0.0.0.0"};
        quint16 m_ReceivePort = 5004;
        QString m_SubscriptionHost{"127.0.0.1"};
        quint16 m_SubscriptionPort = 5004;
    };
}
