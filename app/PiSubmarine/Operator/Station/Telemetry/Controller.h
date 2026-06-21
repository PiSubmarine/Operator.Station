#pragma once

#include <chrono>
#include <functional>
#include <vector>

#include <QObject>

#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Time/ITickable.h"

namespace spdlog
{
    class logger;
}

namespace PiSubmarine::Operator::Station::Telemetry
{
    class LampController;
    class MotorController;
    class BatteryController;
    class BallastController;
    class DepthController;
    class ProximityController;
    class TimeController;
    class VideoStatusController;

    class Controller : public QObject, public ::PiSubmarine::Time::ITickable
    {
        Q_OBJECT

    public:
        Controller(
            LampController& lampController,
            std::vector<std::reference_wrapper<MotorController>> motorControllers,
            BatteryController& batteryController,
            BallastController& ballastController,
            DepthController& depthController,
            ProximityController& proximityController,
            TimeController& timeController,
            VideoStatusController& videoStatusController,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);
        ~Controller() override;

    public slots:
        void Start();
        void Stop();

    private:
        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;
        LampController& m_LampController;
        std::vector<std::reference_wrapper<MotorController>> m_MotorControllers;
        BatteryController& m_BatteryController;
        BallastController& m_BallastController;
        DepthController& m_DepthController;
        ProximityController& m_ProximityController;
        TimeController& m_TimeController;
        VideoStatusController& m_VideoStatusController;
        std::shared_ptr<spdlog::logger> m_Logger;
        bool m_IsStarted = false;
    };
}
