#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"

#include <QMetaObject>
#include <QThread>

#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/BallastController.h"
#include "PiSubmarine/Operator/Station/Telemetry/DepthController.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"
#include "PiSubmarine/Operator/Station/Telemetry/ProximityController.h"
#include "PiSubmarine/Operator/Station/Telemetry/TimeController.h"
#include "PiSubmarine/Operator/Station/Telemetry/VideoStatusController.h"

namespace PiSubmarine::Operator::Station::Telemetry
{
    Controller::Controller(
        LampController& lampController,
        std::vector<std::reference_wrapper<MotorController>> motorControllers,
        BatteryController& batteryController,
        BallastController& ballastController,
        DepthController& depthController,
        ProximityController& proximityController,
        TimeController& timeController,
        VideoStatusController& videoStatusController,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_LampController(lampController)
        , m_MotorControllers(std::move(motorControllers))
        , m_BatteryController(batteryController)
        , m_BallastController(ballastController)
        , m_DepthController(depthController)
        , m_ProximityController(proximityController)
        , m_TimeController(timeController)
        , m_VideoStatusController(videoStatusController)
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

        if (!m_IsStarted)
        {
            return;
        }

        m_IsStarted = false;
    }

    void Controller::Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime)
    {
        static_cast<void>(uptime);
        static_cast<void>(deltaTime);

        if (!m_IsStarted)
        {
            return;
        }

        m_LampController.Refresh();
        for (auto& motorController : m_MotorControllers)
        {
            static_cast<MotorController&>(motorController).Refresh();
        }
        m_BatteryController.Refresh();
        m_BallastController.Refresh();
        m_DepthController.Refresh();
        m_ProximityController.Refresh();
        m_TimeController.Refresh(uptime);
        m_VideoStatusController.Refresh();
    }
}
