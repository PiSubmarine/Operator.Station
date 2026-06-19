#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <vector>

#include <QObject>
#include <QTimer>

#include "PiSubmarine/Error/Api/Error.h"
#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"

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
        Controller(
            ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
            LampController& lampController,
            std::vector<std::reference_wrapper<MotorController>> motorControllers,
            BatteryController& batteryController,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);
        ~Controller() override;

    public slots:
        void Start();
        void Stop();

    private slots:
        void Tick();

    private:
        [[nodiscard]] static bool IsNotReadyError(const Error::Api::Error& error);
        [[nodiscard]] Error::Api::Result<void> EnsureLease(const std::chrono::nanoseconds& uptime);
        void RenewLease(const std::chrono::nanoseconds& uptime);

        ::PiSubmarine::Lease::Api::ILeaseIssuer& m_LeaseIssuer;
        LampController& m_LampController;
        std::vector<std::reference_wrapper<MotorController>> m_MotorControllers;
        BatteryController& m_BatteryController;
        std::shared_ptr<spdlog::logger> m_Logger;
        QTimer m_Timer;
        std::chrono::steady_clock::time_point m_StartTime{};
        std::optional<::PiSubmarine::Lease::Api::Lease> m_Lease;
        std::chrono::nanoseconds m_NextRenewal{0};
        bool m_IsStarted = false;
    };
}
