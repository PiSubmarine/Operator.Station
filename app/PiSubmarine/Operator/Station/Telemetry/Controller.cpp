#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"

#include <QMetaObject>
#include <QThread>
#include <spdlog/spdlog.h>

#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    Controller::Controller(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        LampController& lampController,
        std::vector<std::reference_wrapper<MotorController>> motorControllers,
        BatteryController& batteryController,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_LeaseIssuer(leaseIssuer)
        , m_LampController(lampController)
        , m_MotorControllers(std::move(motorControllers))
        , m_BatteryController(batteryController)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Telemetry.Controller"))
    {
    }

    Controller::~Controller()
    {
        Stop();
    }

    void Controller::Start()
    {
        if (thread() != nullptr && QThread::currentThread() != thread())
        {
            QMetaObject::invokeMethod(this, &Controller::Start, Qt::QueuedConnection);
            return;
        }

        if (m_IsStarted)
        {
            return;
        }

        m_IsStarted = true;
    }

    void Controller::Stop()
    {
        if (thread() != nullptr && QThread::currentThread() != thread())
        {
            if (thread()->isRunning())
            {
                QMetaObject::invokeMethod(this, &Controller::Stop, Qt::BlockingQueuedConnection);
            }
            return;
        }

        if (!m_IsStarted && !m_Lease.has_value())
        {
            return;
        }

        m_IsStarted = false;

        if (m_Lease.has_value())
        {
            static_cast<void>(m_LeaseIssuer.ReleaseLease(m_Lease->Id));
        }

        m_Lease.reset();
        m_NextRenewal = std::chrono::nanoseconds::zero();
    }

    void Controller::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        static_cast<void>(deltaTime);

        if (!m_IsStarted)
        {
            return;
        }

        const auto leaseResult = EnsureLease(uptime);
        if (leaseResult.has_value())
        {
            if (uptime >= m_NextRenewal)
            {
                RenewLease(uptime);
            }

            m_LampController.Refresh();
            for (MotorController& motorController : m_MotorControllers)
            {
                motorController.Refresh();
            }
            m_BatteryController.Refresh();
        }
        else if (!IsNotReadyError(leaseResult.error()))
        {
            SPDLOG_LOGGER_WARN(m_Logger, "Telemetry lease acquisition failed");
        }
    }

    bool Controller::IsNotReadyError(const Error::Api::Error& error)
    {
        return error.Condition == Error::Api::ErrorCondition::NotReady;
    }

    Error::Api::Result<void> Controller::EnsureLease(const std::chrono::nanoseconds& uptime)
    {
        if (m_Lease.has_value())
        {
            return {};
        }

        const auto result = m_LeaseIssuer.AcquireLease({
            .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "telemetry-main"}});

        if (!result.has_value())
        {
            return std::unexpected(result.error());
        }

        m_Lease = result->Lease;
        m_NextRenewal = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_Lease->Duration / 2);
        return {};
    }

    void Controller::RenewLease(const std::chrono::nanoseconds& uptime)
    {
        if (!m_Lease.has_value())
        {
            return;
        }

        const auto result = m_LeaseIssuer.RenewLease(m_Lease->Id);
        if (!result.has_value())
        {
            if (!IsNotReadyError(result.error()))
            {
                m_Lease.reset();
                m_NextRenewal = std::chrono::nanoseconds::zero();
            }
            return;
        }

        m_Lease = *result;
        m_NextRenewal = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_Lease->Duration / 2);
    }
}
