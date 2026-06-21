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

    void Controller::Tick(const std::chrono::nanoseconds&, const std::chrono::nanoseconds&)
    {
        const double surge = m_SurgeAxis != nullptr ? static_cast<double>(m_SurgeAxis->GetValue()) : m_ManualSurge;
        const double yaw = m_YawAxis != nullptr ? static_cast<double>(m_YawAxis->GetValue()) : m_ManualYaw;
        const double ballast = m_BallastAxis != nullptr ? ((static_cast<double>(m_BallastAxis->GetValue()) + 1.0) * 0.5) : m_ManualBallast;
        const double lampIntensity = m_LampAxis != nullptr ? ((static_cast<double>(m_LampAxis->GetValue()) + 1.0) * 0.5) : m_ManualLampIntensity;
        const bool holdPosition = m_HoldPositionKey != nullptr
            ? m_HoldPositionKey->GetState() == ::PiSubmarine::Input::Api::KeyState::Pressed
            : m_ManualHoldPosition;

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
            .ModeRequest = holdPosition
                ? std::optional(::PiSubmarine::Control::Api::Input::Mode::Request::HoldPositionValue())
                : std::optional(::PiSubmarine::Control::Api::Input::Mode::Request::ManualValue())};

        const auto submitResult = m_Sink.Submit(command);
        if (!submitResult.has_value())
        {
            SPDLOG_LOGGER_WARN(m_Logger, "Failed to submit operator intent");
        }
    }

    void Controller::SubmitIntent(
        const double surge,
        const double yaw,
        const double ballast,
        const double lampIntensity,
        const bool holdPosition)
    {
        m_ManualSurge = surge;
        m_ManualYaw = yaw;
        m_ManualBallast = ballast;
        m_ManualLampIntensity = lampIntensity;
        m_ManualHoldPosition = holdPosition;
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

    void Controller::SetHoldPositionKey(::PiSubmarine::Input::Api::IKey* key)
    {
        m_HoldPositionKey = key;
        SPDLOG_LOGGER_INFO(m_Logger, "Bound input path 'Hold Position'");
    }
}
