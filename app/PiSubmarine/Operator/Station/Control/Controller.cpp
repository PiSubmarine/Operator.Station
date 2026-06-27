#include "PiSubmarine/Operator/Station/Control/Controller.h"

#include <chrono>
#include <memory>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Control/Api/Input/Mode/Request.h"
#include "PiSubmarine/Control/Api/Input/OperatorCommand.h"
#include "PiSubmarine/Ballast/BallastFillFraction.h"
#include "PiSubmarine/Control/Horizontal/Api/Command.h"
#include "PiSubmarine/Control/Lamp/Api/Command.h"
#include "PiSubmarine/Control/Vertical/Api/Command.h"
#include "PiSubmarine/Error/Api/ToStringView.h"
#include "PiSubmarine/SignedNormalizedFraction.h"

namespace PiSubmarine::Operator::Station::Control
{
    Controller::Controller(
        ::PiSubmarine::Control::Api::Input::ISink& sink,
        const Config& config,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_Sink(sink)
        , m_Config(config)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Control.Controller"))
    {
        if (m_Config.MinimumGimbalPitch.Value > m_Config.MaximumGimbalPitch.Value)
        {
            std::swap(m_Config.MinimumGimbalPitch, m_Config.MaximumGimbalPitch);
        }
    }

    double Controller::ClampLampIntensity(const double value)
    {
        return std::clamp(value, 0.0, 1.0);
    }

    double Controller::ClampNormalizedValue(const double value)
    {
        return std::clamp(value, 0.0, 1.0);
    }

    double Controller::ClampNonNegative(const double value)
    {
        return std::max(value, 0.0);
    }

    double Controller::ReadAxisValue(const ::PiSubmarine::Input::Api::IAxis* axis)
    {
        if (axis == nullptr)
        {
            return 0.0;
        }

        return static_cast<double>(axis->GetValue());
    }

    Degrees Controller::ClampGimbalPitch(const Degrees value) const
    {
        return Degrees(std::clamp(
            value.Value,
            m_Config.MinimumGimbalPitch.Value,
            m_Config.MaximumGimbalPitch.Value));
    }

    void Controller::Tick(const std::chrono::nanoseconds&, const std::chrono::nanoseconds& deltaTime)
    {
        const auto deltaSeconds = std::chrono::duration<double>(deltaTime).count();

        if (m_LampAxis != nullptr)
        {
            const auto nextLampIntensity = ClampLampIntensity(
                m_DesiredLampIntensity +
                ReadAxisValue(m_LampAxis) * m_Config.LampIntensityChangeSpeedPerSecond * deltaSeconds);
            if (nextLampIntensity != m_DesiredLampIntensity)
            {
                m_DesiredLampIntensity = nextLampIntensity;
                emit LampIntentChanged(m_DesiredLampIntensity);
            }
        }

        if (m_GimbalPitchAxis != nullptr)
        {
            const auto nextGimbalPitch = ClampGimbalPitch(
                m_DesiredGimbalPitch +
                Degrees(ReadAxisValue(m_GimbalPitchAxis) * m_Config.GimbalPitchChangeSpeed.Value * deltaSeconds));
            if (nextGimbalPitch != m_DesiredGimbalPitch)
            {
                m_DesiredGimbalPitch = nextGimbalPitch;
                emit GimbalIntentChanged(m_DesiredGimbalPitch.Value);
            }
        }

        if (m_GimbalCenterKey != nullptr)
        {
            const auto gimbalCenterPressed =
                m_GimbalCenterKey->GetState() == ::PiSubmarine::Input::Api::KeyState::Pressed;
            if (!m_HasGimbalCenterKeySnapshot || gimbalCenterPressed != m_LastGimbalCenterKeyState)
            {
                m_LastGimbalCenterKeyState = gimbalCenterPressed;
                m_HasGimbalCenterKeySnapshot = true;

                if (gimbalCenterPressed && m_DesiredGimbalPitch.Value != 0.0)
                {
                    m_DesiredGimbalPitch = Degrees{0.0};
                    emit GimbalIntentChanged(m_DesiredGimbalPitch.Value);
                }
            }
        }

        if (m_BallastAxis != nullptr)
        {
            const auto ballastAxisValue = ReadAxisValue(m_BallastAxis);
            if (ballastAxisValue != 0.0)
            {
                auto verticalIntentChanged = false;

                if (m_VerticalMode == VerticalMode::SetBallastPosition)
                {
                    const auto nextBallastPosition = ClampNormalizedValue(
                        m_DesiredBallastPosition +
                        ballastAxisValue * m_Config.BallastPositionChangeSpeedPerSecond * deltaSeconds);
                    if (nextBallastPosition != m_DesiredBallastPosition)
                    {
                        m_DesiredBallastPosition = nextBallastPosition;
                        verticalIntentChanged = true;
                    }
                }
                else if (m_VerticalMode == VerticalMode::SetDepthTarget)
                {
                    const auto nextDepthTargetMeters = ClampNonNegative(
                        m_DesiredDepthTargetMeters +
                        ballastAxisValue * m_Config.DepthTargetChangeSpeedMetersPerSecond * deltaSeconds);
                    if (nextDepthTargetMeters != m_DesiredDepthTargetMeters)
                    {
                        m_DesiredDepthTargetMeters = nextDepthTargetMeters;
                        verticalIntentChanged = true;
                    }
                }

                if (verticalIntentChanged)
                {
                    emit VerticalIntentChanged(
                        static_cast<int>(m_VerticalMode),
                        m_DesiredBallastPosition,
                        m_DesiredDepthTargetMeters);
                }
            }
        }

        const double surge = m_SurgeAxis != nullptr ? static_cast<double>(m_SurgeAxis->GetValue()) : 0.0;
        const double yaw = m_YawAxis != nullptr ? static_cast<double>(m_YawAxis->GetValue()) : 0.0;
        const double lampIntensity = m_DesiredLampIntensity;

        const auto horizontalResult = ::PiSubmarine::Control::Horizontal::Api::Command::Create(
            SignedNormalizedFraction(surge),
            SignedNormalizedFraction(yaw));
        if (!horizontalResult.has_value())
        {
            SPDLOG_LOGGER_WARN(m_Logger, "Rejected invalid movement input");
            return;
        }

        ::PiSubmarine::Control::Api::Input::OperatorCommand command{
            .Movement = *horizontalResult,
            .VerticalControl = m_VerticalMode == VerticalMode::KeepCurrent
                ? ::PiSubmarine::Control::Vertical::Api::Command::KeepCurrentValue()
                : m_VerticalMode == VerticalMode::SetBallastPosition
                    ? ::PiSubmarine::Control::Vertical::Api::Command::SetBallastPositionTo(
                        ::PiSubmarine::Ballast::BallastFillFraction{
                            ::PiSubmarine::NormalizedFraction{m_DesiredBallastPosition}})
                    : ::PiSubmarine::Control::Vertical::Api::Command::SetDepthTargetTo(
                        Meters{m_DesiredDepthTargetMeters}),
            .GimbalTarget = std::optional(::PiSubmarine::Control::Gimbal::Api::Command::Create(
                m_DesiredGimbalPitch.ToRadians())),
            .LampIntensity = ::PiSubmarine::Control::Lamp::Api::Command::Create(
                NormalizedFraction(lampIntensity)),
            .VideoControl = m_IsCameraEnabled
                ? std::optional(::PiSubmarine::Control::Video::Api::Command::Enable(
                    m_StreamProfile,
                    m_IsAutoFocusEnabled
                    ? ::PiSubmarine::Control::Video::Api::FocusMode{::PiSubmarine::Control::Video::Api::AutoFocus{}}
                        : ::PiSubmarine::Control::Video::Api::FocusMode{
                            ::PiSubmarine::Control::Video::Api::ManualFocus{
                                .Position = NormalizedFraction(m_ManualFocusPosition)}}))
                : std::optional(::PiSubmarine::Control::Video::Api::Command::Disable()),
            .ModeRequest = m_DesiredHoldPosition
                ? std::optional(::PiSubmarine::Control::Api::Input::Mode::Request::HoldPositionValue())
                : std::optional(::PiSubmarine::Control::Api::Input::Mode::Request::ManualValue())};

        if (!m_HasPublishedCameraIntent)
        {
            m_HasPublishedCameraIntent = true;
            emit CameraIntentChanged(
                m_IsCameraEnabled,
                m_IsAutoFocusEnabled,
                m_ManualFocusPosition,
                static_cast<int>(m_StreamProfile));
        }

        if (!m_HasPublishedGimbalIntent)
        {
            m_HasPublishedGimbalIntent = true;
            emit GimbalIntentChanged(m_DesiredGimbalPitch.Value);
        }

        if (!m_HasPublishedModeIntent)
        {
            m_HasPublishedModeIntent = true;
            emit ModeIntentChanged(m_DesiredHoldPosition);
        }

        if (!m_HasPublishedVerticalIntent)
        {
            m_HasPublishedVerticalIntent = true;
            emit VerticalIntentChanged(
                static_cast<int>(m_VerticalMode),
                m_DesiredBallastPosition,
                m_DesiredDepthTargetMeters);
        }

        const auto submitResult = m_Sink.Submit(command);
        if (!submitResult.has_value())
        {
            const auto& error = submitResult.error();
            if (error.HasCause())
            {
                SPDLOG_LOGGER_WARN(
                    m_Logger,
                    "Failed to submit operator intent: {} ({}: {})",
                    ::PiSubmarine::Error::Api::ToStringView(error.Condition),
                    error.Cause.value(),
                    error.Cause.message());
            }
            else
            {
                SPDLOG_LOGGER_WARN(
                    m_Logger,
                    "Failed to submit operator intent: {}",
                    ::PiSubmarine::Error::Api::ToStringView(error.Condition));
            }
        }
    }

    void Controller::LampChangeIntensity(const double step)
    {
        const auto nextLampIntensity = ClampLampIntensity(m_DesiredLampIntensity + step);
        if (m_DesiredLampIntensity == nextLampIntensity)
        {
            return;
        }

        m_DesiredLampIntensity = nextLampIntensity;
        emit LampIntentChanged(m_DesiredLampIntensity);
    }

    void Controller::SetSurgeAxis(::PiSubmarine::Input::Api::IAxis* axis)
    {
        m_SurgeAxis = axis;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Surge'");
    }

    void Controller::SetYawAxis(::PiSubmarine::Input::Api::IAxis* axis)
    {
        m_YawAxis = axis;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Yaw'");
    }

    void Controller::SetBallastAxis(::PiSubmarine::Input::Api::IAxis* axis)
    {
        m_BallastAxis = axis;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Ballast'");
    }

    void Controller::SetLampAxis(::PiSubmarine::Input::Api::IAxis* axis)
    {
        m_LampAxis = axis;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Lamp'");
    }

    void Controller::SetGimbalPitchAxis(::PiSubmarine::Input::Api::IAxis* axis)
    {
        m_GimbalPitchAxis = axis;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Gimbal Pitch'");
    }

    void Controller::SetGimbalCenterKey(::PiSubmarine::Input::Api::IKey* key)
    {
        m_GimbalCenterKey = key;
        m_HasGimbalCenterKeySnapshot = false;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Gimbal Center'");
    }

    void Controller::EnableCamera()
    {
        if (m_IsCameraEnabled)
        {
            return;
        }

        m_IsCameraEnabled = true;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::DisableCamera()
    {
        if (!m_IsCameraEnabled)
        {
            return;
        }

        m_IsCameraEnabled = false;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::SetAutoFocusEnabled()
    {
        if (m_IsAutoFocusEnabled)
        {
            return;
        }

        m_IsAutoFocusEnabled = true;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::SetManualFocusEnabled()
    {
        if (!m_IsAutoFocusEnabled)
        {
            return;
        }

        m_IsAutoFocusEnabled = false;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::SetManualFocusPosition(const double position)
    {
        const auto nextPosition = ClampNormalizedValue(position);
        if (m_ManualFocusPosition == nextPosition)
        {
            return;
        }

        m_ManualFocusPosition = nextPosition;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::SetLowQualityStreamProfile()
    {
        if (m_StreamProfile == ::PiSubmarine::Control::Video::Api::StreamProfile::LowLatency)
        {
            return;
        }

        m_StreamProfile = ::PiSubmarine::Control::Video::Api::StreamProfile::LowLatency;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::SetMediumQualityStreamProfile()
    {
        if (m_StreamProfile == ::PiSubmarine::Control::Video::Api::StreamProfile::Standard)
        {
            return;
        }

        m_StreamProfile = ::PiSubmarine::Control::Video::Api::StreamProfile::Standard;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::SetHighQualityStreamProfile()
    {
        if (m_StreamProfile == ::PiSubmarine::Control::Video::Api::StreamProfile::HighQuality)
        {
            return;
        }

        m_StreamProfile = ::PiSubmarine::Control::Video::Api::StreamProfile::HighQuality;
        emit CameraIntentChanged(m_IsCameraEnabled, m_IsAutoFocusEnabled, m_ManualFocusPosition, static_cast<int>(m_StreamProfile));
    }

    void Controller::SetManualMode()
    {
        if (!m_DesiredHoldPosition)
        {
            return;
        }

        m_DesiredHoldPosition = false;
        emit ModeIntentChanged(false);
    }

    void Controller::SetHoldPositionMode()
    {
        if (m_DesiredHoldPosition)
        {
            return;
        }

        m_DesiredHoldPosition = true;
        emit ModeIntentChanged(true);
    }

    void Controller::SetVerticalKeepCurrentMode()
    {
        if (m_VerticalMode == VerticalMode::KeepCurrent)
        {
            return;
        }

        m_VerticalMode = VerticalMode::KeepCurrent;
        emit VerticalIntentChanged(
            static_cast<int>(m_VerticalMode),
            m_DesiredBallastPosition,
            m_DesiredDepthTargetMeters);
    }

    void Controller::SetVerticalBallastPositionMode()
    {
        if (m_VerticalMode == VerticalMode::SetBallastPosition)
        {
            return;
        }

        m_VerticalMode = VerticalMode::SetBallastPosition;
        emit VerticalIntentChanged(
            static_cast<int>(m_VerticalMode),
            m_DesiredBallastPosition,
            m_DesiredDepthTargetMeters);
    }

    void Controller::SetVerticalDepthTargetMode()
    {
        if (m_VerticalMode == VerticalMode::SetDepthTarget)
        {
            return;
        }

        m_VerticalMode = VerticalMode::SetDepthTarget;
        emit VerticalIntentChanged(
            static_cast<int>(m_VerticalMode),
            m_DesiredBallastPosition,
            m_DesiredDepthTargetMeters);
    }
}
