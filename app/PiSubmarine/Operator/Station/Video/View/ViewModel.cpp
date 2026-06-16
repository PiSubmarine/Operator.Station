#include "PiSubmarine/Operator/Station/Video/View/ViewModel.h"

namespace PiSubmarine::Operator::Station::Video::View
{
    ViewModel::ViewModel(QObject* parent)
        : QObject(parent)
    {
    }

    QString ViewModel::GetReceiveBindAddress() const
    {
        return m_ReceiveBindAddress;
    }

    void ViewModel::SetReceiveBindAddress(const QString& receiveBindAddress)
    {
        if (m_ReceiveBindAddress == receiveBindAddress)
        {
            return;
        }

        m_ReceiveBindAddress = receiveBindAddress;
        emit ReceiveEndpointChanged(m_ReceiveBindAddress, m_ReceivePort);
    }

    quint16 ViewModel::GetReceivePort() const
    {
        return m_ReceivePort;
    }

    void ViewModel::SetReceivePort(const quint16 receivePort)
    {
        if (m_ReceivePort == receivePort)
        {
            return;
        }

        m_ReceivePort = receivePort;
        emit ReceiveEndpointChanged(m_ReceiveBindAddress, m_ReceivePort);
    }

    QString ViewModel::GetSubscriptionHost() const
    {
        return m_SubscriptionHost;
    }

    void ViewModel::SetSubscriptionHost(const QString& subscriptionHost)
    {
        if (m_SubscriptionHost == subscriptionHost)
        {
            return;
        }

        m_SubscriptionHost = subscriptionHost;
        emit SubscriptionEndpointChanged(m_SubscriptionHost, m_SubscriptionPort);
    }

    quint16 ViewModel::GetSubscriptionPort() const
    {
        return m_SubscriptionPort;
    }

    void ViewModel::SetSubscriptionPort(const quint16 subscriptionPort)
    {
        if (m_SubscriptionPort == subscriptionPort)
        {
            return;
        }

        m_SubscriptionPort = subscriptionPort;
        emit SubscriptionEndpointChanged(m_SubscriptionHost, m_SubscriptionPort);
    }
}
