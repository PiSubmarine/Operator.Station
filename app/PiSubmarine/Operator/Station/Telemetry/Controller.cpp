#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"

#include <spdlog/spdlog.h>

#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    Controller::Controller(
        ::PiSubmarine::Lease::Api::ILeaseIssuer& leaseIssuer,
        ISubscriptionService& subscriptionService,
        LampController& lampController,
        MotorController& motorController,
        BatteryController& batteryController,
        Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_LeaseIssuer(leaseIssuer)
        , m_SubscriptionService(subscriptionService)
        , m_LampController(lampController)
        , m_MotorController(motorController)
        , m_BatteryController(batteryController)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Telemetry.Controller"))
    {
        m_Timer.setParent(this);
        m_Timer.setInterval(200);
        connect(&m_Timer, &QTimer::timeout, this, &Controller::Tick);
    }

    void Controller::Start()
    {
        if (m_IsStarted)
        {
            return;
        }

        m_IsStarted = true;
        m_StartTime = std::chrono::steady_clock::now();
        m_Timer.start();
    }

    void Controller::Stop()
    {
        // TODO m_Timer is moved to another thread, but stopped in UI thread.
        m_Timer.stop();
        m_IsStarted = false;

        if (m_IsSubscribed && m_Lease.has_value())
        {
            static_cast<void>(m_SubscriptionService.Unsubscribe(m_Lease->Id));
        }

        if (m_Lease.has_value())
        {
            static_cast<void>(m_LeaseIssuer.ReleaseLease(m_Lease->Id));
        }

        m_Lease.reset();
        m_IsSubscribed = false;
    }

    void Controller::SetSubscriptionEndpoint(const QString& host, const quint16 port)
    {
        const Shared::Endpoint nextEndpoint{host.toStdString(), port};
        if (m_SubscriptionEndpoint == nextEndpoint)
        {
            return;
        }

        m_SubscriptionEndpoint = nextEndpoint;
        m_IsSubscribed = false;
    }

    void Controller::Tick()
    {
        if (!m_IsStarted)
        {
            return;
        }

        const auto uptime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - m_StartTime);

        if (EnsureLease(uptime))
        {
            if (uptime >= m_NextRenewal)
            {
                RenewLease(uptime);
            }

            EnsureSubscription();
            m_LampController.Refresh();
            m_MotorController.Refresh();
            m_BatteryController.Refresh();
        }
    }

    // TODO WTF, Use Error::Api::ErrorCondition::NotReady directly.
    Error::Api::ErrorCondition Controller::GetNotReadyCondition()
    {
        return static_cast<Error::Api::ErrorCondition>(3);
    }

    bool Controller::EnsureLease(const std::chrono::nanoseconds& uptime)
    {
        if (m_Lease.has_value())
        {
            return true;
        }

        const auto result = m_LeaseIssuer.AcquireLease({
            .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "telemetry-main"}});

        if (!result.has_value())
        {
            // TODO current code does not distinguish between denial and temporary unavailability. All errors are reported as "Not Ready", which is inaccurate.
            return result.error().Condition == GetNotReadyCondition();
        }

        m_Lease = result->Lease;
        m_NextRenewal = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_Lease->Duration / 2);
        return true;
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
            if (result.error().Condition != GetNotReadyCondition())
            {
                m_Lease.reset();
                m_IsSubscribed = false;
            }
            return;
        }

        m_Lease = *result;
        m_NextRenewal = uptime + std::chrono::duration_cast<std::chrono::nanoseconds>(m_Lease->Duration / 2);
    }

    // TODO not needed
    void Controller::EnsureSubscription()
    {
        if (m_IsSubscribed || !m_Lease.has_value())
        {
            return;
        }

        const auto result = m_SubscriptionService.Subscribe(m_Lease->Id, m_SubscriptionEndpoint);
        if (result.has_value())
        {
            m_IsSubscribed = true;
        }
    }
}
