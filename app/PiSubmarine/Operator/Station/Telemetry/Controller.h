#pragma once

#include <chrono>
#include <optional>

#include <QObject>
#include <QTimer>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Shared/Endpoint.h"
#include "PiSubmarine/Operator/Station/Telemetry/ISubscriptionService.h"

namespace spdlog
{
    class logger;
}

namespace PiSubmarine::Operator::Station::Telemetry
{
    class LampController;
    class MotorController;
    class BatteryController;

    class Controller : public QObject
    {
        Q_OBJECT

    public:
        // TODO PiSubmarine::Operator::Station::Telemetry::ISubscriptionService is not needed. Subscription is maintained internally by Telemetry.Client.Udp. Remove ISubscriptionService type completely.
        Controller(
            ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
            ISubscriptionService& subscriptionService,
            LampController& lampController,
            // TODO There are 4 motors in the drone currently. Allow construction of a variable number of MotorControllers and associated Views.
            MotorController& motorController,
            BatteryController& batteryController,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);

    public slots:
        void Start();
        void Stop();
        void SetSubscriptionEndpoint(const QString& host, quint16 port);

    private slots:
        void Tick();

    private:
        [[nodiscard]] static Error::Api::ErrorCondition GetNotReadyCondition();
        [[nodiscard]] bool EnsureLease(const std::chrono::nanoseconds& uptime);
        void RenewLease(const std::chrono::nanoseconds& uptime);
        void EnsureSubscription();

        ::PiSubmarine::Lease::Api::ILeaseIssuer& m_LeaseIssuer;
        ISubscriptionService& m_SubscriptionService;
        LampController& m_LampController;
        MotorController& m_MotorController;
        BatteryController& m_BatteryController;
        std::shared_ptr<spdlog::logger> m_Logger;
        QTimer m_Timer;
        std::chrono::steady_clock::time_point m_StartTime{};
        std::optional<::PiSubmarine::Lease::Api::Lease> m_Lease;
        Shared::Endpoint m_SubscriptionEndpoint{"127.0.0.1", 6100};
        std::chrono::nanoseconds m_NextRenewal{0};
        bool m_IsSubscribed = false;
        bool m_IsStarted = false;
    };
}
