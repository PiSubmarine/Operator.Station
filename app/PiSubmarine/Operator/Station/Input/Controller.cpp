#include "PiSubmarine/Operator/Station/Input/Controller.h"

#include <memory>

#include <spdlog/spdlog.h>

#include "PiSubmarine/Control/Api/Input/Mode/Request.h"
#include "PiSubmarine/Control/Api/Input/OperatorCommand.h"
#include "PiSubmarine/Control/Horizontal/Api/Command.h"
#include "PiSubmarine/Control/Lamp/Api/Command.h"
#include "PiSubmarine/Control/Vertical/Api/Command.h"
#include "PiSubmarine/Operator/Station/Input/View/ViewModel.h"
#include "PiSubmarine/SignedNormalizedFraction.h"

namespace PiSubmarine::Operator::Station::Input
{
    Controller::Controller(
        ::PiSubmarine::Control::Api::Input::ISink& sink,
        View::ViewModel& viewModel,
        Logging::Api::IFactory& loggerFactory,
        QObject* parent)
        : QObject(parent)
        , m_Sink(sink)
        , m_ViewModel(viewModel)
        , m_Logger(loggerFactory.CreateLogger("Operator.Station.Input.Controller"))
    {
    }

    void Controller::SubmitCurrentIntent()
    {
        const auto horizontalResult = ::PiSubmarine::Control::Horizontal::Api::Command::Create(
            SignedNormalizedFraction(m_ViewModel.GetSurge()),
            SignedNormalizedFraction(m_ViewModel.GetYaw()));
        if (!horizontalResult.has_value())
        {
            SPDLOG_LOGGER_WARN(m_Logger, "Rejected invalid movement input");
            return;
        }

        ::PiSubmarine::Control::Api::Input::OperatorCommand command{
            .LeaseId = ::PiSubmarine::Lease::Api::LeaseId{.Value = "operator-station-input"},
            .Movement = *horizontalResult,
            .VerticalControl = ::PiSubmarine::Control::Vertical::Api::Command::SetBallastPositionTo(
                NormalizedFraction(m_ViewModel.GetBallast())),
            .LampIntensity = ::PiSubmarine::Control::Lamp::Api::Command::Create(
                NormalizedFraction(m_ViewModel.GetLampIntensity())),
            .ModeRequest = m_ViewModel.GetHoldPosition()
                ? std::optional(::PiSubmarine::Control::Api::Input::Mode::Request::HoldPositionValue())
                : std::optional(::PiSubmarine::Control::Api::Input::Mode::Request::ManualValue())};

        const auto submitResult = m_Sink.Submit(command);
        if (!submitResult.has_value())
        {
            SPDLOG_LOGGER_WARN(m_Logger, "Failed to submit operator intent");
        }
    }
}
