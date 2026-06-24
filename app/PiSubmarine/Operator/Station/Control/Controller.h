#pragma once

#include <algorithm>
#include <chrono>
#include <memory>

#include <QObject>

#include "PiSubmarine/Input/Api/IAxis.h"
#include "PiSubmarine/Input/Api/IKey.h"
#include "PiSubmarine/Control/Api/Input/ISink.h"
#include "PiSubmarine/Degrees.h"
#include "PiSubmarine/Control/Video/Api/Command.h"
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
        struct Config
        {
            double LampIntensityChangeSpeedPerSecond = 0.25;
            Degrees GimbalPitchChangeSpeed = Degrees{45.0};
            Degrees MinimumGimbalPitch = Degrees{-90.0};
            Degrees MaximumGimbalPitch = Degrees{90.0};
            double BallastPositionChangeSpeedPerSecond = 0.1;
            double DepthTargetChangeSpeedMetersPerSecond = 0.25;
        };

        Controller(
            ::PiSubmarine::Control::Api::Input::ISink& sink,
            const Config& config,
            PiSubmarine::Logging::Api::IFactory& loggerFactory,
            QObject* parent = nullptr);

        void Tick(const std::chrono::nanoseconds& uptime, const std::chrono::nanoseconds& deltaTime) override;

    public slots:
        void LampChangeIntensity(double step);
        void SetSurgeAxis(PiSubmarine::Input::Api::IAxis* axis);
        void SetYawAxis(PiSubmarine::Input::Api::IAxis* axis);
        void SetBallastAxis(PiSubmarine::Input::Api::IAxis* axis);
        void SetLampAxis(PiSubmarine::Input::Api::IAxis* axis);
        void SetGimbalPitchAxis(PiSubmarine::Input::Api::IAxis* axis);
        void SetGimbalCenterKey(PiSubmarine::Input::Api::IKey* key);

    signals:
        void LampIntentChanged(double lampIntensity);
        void CameraIntentChanged(bool isEnabled, bool isAutoFocus, double focusPosition, int streamProfile);
        void GimbalIntentChanged(double pitchDegrees);
        void ModeIntentChanged(bool isHoldPosition);
        void VerticalIntentChanged(int verticalMode, double ballastPosition, double depthTargetMeters);

    private:
        enum class VerticalMode
        {
            KeepCurrent = 0,
            SetBallastPosition = 1,
            SetDepthTarget = 2
        };

        [[nodiscard]] static double ClampLampIntensity(double value);
        [[nodiscard]] static double ClampNormalizedValue(double value);
        [[nodiscard]] static double ClampNonNegative(double value);
        [[nodiscard]] static double ReadAxisValue(const ::PiSubmarine::Input::Api::IAxis* axis);
        [[nodiscard]] Degrees ClampGimbalPitch(Degrees value) const;

        ::PiSubmarine::Control::Api::Input::ISink& m_Sink;
        Config m_Config;
        std::shared_ptr<spdlog::logger> m_Logger;
        ::PiSubmarine::Input::Api::IAxis* m_SurgeAxis = nullptr;
        ::PiSubmarine::Input::Api::IAxis* m_YawAxis = nullptr;
        ::PiSubmarine::Input::Api::IAxis* m_BallastAxis = nullptr;
        ::PiSubmarine::Input::Api::IAxis* m_LampAxis = nullptr;
        ::PiSubmarine::Input::Api::IAxis* m_GimbalPitchAxis = nullptr;
        ::PiSubmarine::Input::Api::IKey* m_GimbalCenterKey = nullptr;
        double m_DesiredLampIntensity = 0.0;
        Degrees m_DesiredGimbalPitch = Degrees{0.0};
        bool m_IsCameraEnabled = true;
        bool m_IsAutoFocusEnabled = true;
        double m_ManualFocusPosition = 0.5;
        ::PiSubmarine::Control::Video::Api::StreamProfile m_StreamProfile =
            ::PiSubmarine::Control::Video::Api::StreamProfile::Standard;
        bool m_HasPublishedCameraIntent = false;
        bool m_HasPublishedGimbalIntent = false;
        bool m_DesiredHoldPosition = false;
        VerticalMode m_VerticalMode = VerticalMode::SetBallastPosition;
        double m_DesiredBallastPosition = 0.5;
        double m_DesiredDepthTargetMeters = 0.0;
        bool m_LastGimbalCenterKeyState = false;
        bool m_HasGimbalCenterKeySnapshot = false;
        bool m_HasPublishedModeIntent = false;
        bool m_HasPublishedVerticalIntent = false;

    public slots:
        void EnableCamera();
        void DisableCamera();
        void SetAutoFocusEnabled();
        void SetManualFocusEnabled();
        void SetManualFocusPosition(double position);
        void SetLowQualityStreamProfile();
        void SetMediumQualityStreamProfile();
        void SetHighQualityStreamProfile();
        void SetManualMode();
        void SetHoldPositionMode();
        void SetVerticalKeepCurrentMode();
        void SetVerticalBallastPositionMode();
        void SetVerticalDepthTargetMode();
    };
}
