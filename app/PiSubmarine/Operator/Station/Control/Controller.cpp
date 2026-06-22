#include "PiSubmarine/Operator/Station/Control/Controller.h"

#include <memory>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Control/Api/Input/Mode/Request.h"
#include "PiSubmarine/Control/Api/Input/OperatorCommand.h"
#include "PiSubmarine/Control/Horizontal/Api/Command.h"
#include "PiSubmarine/Control/Lamp/Api/Command.h"
#include "PiSubmarine/Control/Vertical/Api/Command.h"
#include "PiSubmarine/SignedNormalizedFraction.h"

namespace PiSubmarine::Operator::Station::Control
{
    Controller::Controller(
        ::PiSubmarine::Control::Api::Input::ISink& sink,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_Sink(sink)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Control.Controller"))
    {
    }

    double Controller::ClampLampIntensity(const double value)
    {
        return std::clamp(value, 0.0, 1.0);
    }

    double Controller::ClampNormalizedValue(const double value)
    {
        return std::clamp(value, 0.0, 1.0);
    }

    double Controller::ReadLampAxisIntensity() const
    {
        if (m_LampAxis == nullptr)
        {
            return 0.0;
        }

        return ClampLampIntensity((static_cast<double>(m_LampAxis->GetValue()) + 1.0) * 0.5);
    }

    void Controller::Tick(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)
    {
        if (m_LampAxis != nullptr)
        {
            const auto lampAxisIntensity = ReadLampAxisIntensity();
            if (!m_HasLampAxisSnapshot || lampAxisIntensity != m_LastLampAxisIntensity)
            {
                m_LastLampAxisIntensity = lampAxisIntensity;
                m_HasLampAxisSnapshot = true;
                m_LastLampInputSource = LampInputSource::Axis;

                if (m_DesiredLampIntensity != lampAxisIntensity)
                {
                    m_DesiredLampIntensity = lampAxisIntensity;
                    emit LampIntentChanged(m_DesiredLampIntensity);
                }
            }
        }

        const double surge = m_SurgeAxis != nullptr ? static_cast<double>(m_SurgeAxis->GetValue()) : 0.0;
        const double yaw = m_YawAxis != nullptr ? static_cast<double>(m_YawAxis->GetValue()) : 0.0;
        const double ballast = m_BallastAxis != nullptr ? ((static_cast<double>(m_BallastAxis->GetValue()) + 1.0) * 0.5) : 0.5;
        const double lampIntensity = m_DesiredLampIntensity;
        const bool holdPosition = m_HoldPositionKey != nullptr
            ? m_HoldPositionKey->GetState() == ::PiSubmarine::Input::Api::KeyState::Pressed
            : false;

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
            .VerticalControl = ::PiSubmarine::Control::Vertical::Api::Command::SetBallastPositionTo(
                NormalizedFraction(ballast)),
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
            .ModeRequest = holdPosition
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

        const auto submitResult = m_Sink.Submit(command);
        if (!submitResult.has_value())
        {
            SPDLOG_LOGGER_WARN(m_Logger, "Failed to submit operator intent");
        }
    }

    void Controller::LampChangeIntensity(const double step)
    {
        const auto nextLampIntensity = ClampLampIntensity(m_DesiredLampIntensity + step);
        if (m_DesiredLampIntensity == nextLampIntensity && m_LastLampInputSource == LampInputSource::Ui)
        {
            return;
        }

        m_DesiredLampIntensity = nextLampIntensity;
        m_LastLampInputSource = LampInputSource::Ui;
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
        m_HasLampAxisSnapshot = false;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Lamp'");
    }

    void Controller::SetHoldPositionKey(::PiSubmarine::Input::Api::IKey* key)
    {
        m_HoldPositionKey = key;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Hold Position'");
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
}
