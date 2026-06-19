#include "PiSubmarine/Operator/Station/Input/Controller.h"

#include <memory>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Control/Api/Input/Mode/Request.h"
#include "PiSubmarine/Control/Api/Input/OperatorCommand.h"
#include "PiSubmarine/Control/Horizontal/Api/Command.h"
#include "PiSubmarine/Control/Lamp/Api/Command.h"
#include "PiSubmarine/Control/Vertical/Api/Command.h"
#include "PiSubmarine/SignedNormalizedFraction.h"

namespace PiSubmarine::Operator::Station::Input
{
    Controller::Controller(
        ::PiSubmarine::Control::Api::Input::ISink& sink,
        PiSubmarine::Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_Sink(sink)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Input.Controller"))
    {
    }

    void Controller::SubmitIntent(
        const double surge,
        const double yaw,
        const double ballast,
        const double lampIntensity,
        const bool holdPosition)
    {
        const auto horizontalResult = ::PiSubmarine::Control::Horizontal::Api::Command::Create(
            SignedNormalizedFraction(surge),
            SignedNormalizedFraction(yaw));
        if (!horizontalResult.has_value())
        {
            SPDLOG_LOGGER_WARN(m_Logger, "Rejected invalid movement input");
            return;
        }

        ::PiSubmarine::Control::Api::Input::OperatorCommand command{
            .LeaseId = ::PiSubmarine::Lease::Api::LeaseId{.Value = "operator-station-input"},
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
}
