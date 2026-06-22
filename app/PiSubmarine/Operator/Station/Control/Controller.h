#pragma once

#include <algorithm>
#include <chrono>
#include <memory>

#include <QObject>

#include "PiSubmarine/Input/Api/IAxis.h"
#include "PiSubmarine/Input/Api/IKey.h"
#include "PiSubmarine/Control/Api/Input/ISink.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Time/ITickable.h"

namespace spdlog
{
    class logger;
}

namespace PiSubmarine::Operator::Station::Control
{
    class Controller final : public QObject, public ::PiSubmarine::Time::ITickable
    {
        Q_OBJECT

    public:
        Controller(
            ::PiSubmarine::Control::Api::Input::ISink& sink,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);

        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    public slots:
        void LampChangeIntensity(double step);
        void SetSurgeAxis(::PiSubmarine::Input::Api::IAxis* axis);
        void SetYawAxis(::PiSubmarine::Input::Api::IAxis* axis);
        void SetBallastAxis(::PiSubmarine::Input::Api::IAxis* axis);
        void SetLampAxis(::PiSubmarine::Input::Api::IAxis* axis);
        void SetHoldPositionKey(::PiSubmarine::Input::Api::IKey* key);

    signals:
        void LampIntentChanged(double lampIntensity);

    private:
        enum class LampInputSource
        {
            Ui,
            Axis
        };

        [[nodiscard]] static double ClampLampIntensity(double value);
        [[nodiscard]] double ReadLampAxisIntensity() const;

        ::PiSubmarine::Control::Api::Input::ISink& m_Sink;
        std::shared_ptr<spdlog::logger> m_Logger;
        ::PiSubmarine::Input::Api::IAxis* m_SurgeAxis = nullptr;
        ::PiSubmarine::Input::Api::IAxis* m_YawAxis = nullptr;
        ::PiSubmarine::Input::Api::IAxis* m_BallastAxis = nullptr;
        ::PiSubmarine::Input::Api::IAxis* m_LampAxis = nullptr;
        ::PiSubmarine::Input::Api::IKey* m_HoldPositionKey = nullptr;
        double m_DesiredLampIntensity = 0.0;
        double m_LastLampAxisIntensity = 0.0;
        bool m_HasLampAxisSnapshot = false;
        LampInputSource m_LastLampInputSource = LampInputSource::Ui;
    };
}
