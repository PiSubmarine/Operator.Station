#include <chrono>
#include <optional>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>

#include "PiSubmarine/Control/Api/Input/ISinkMock.h"
#include "PiSubmarine/Control/Vertical/Api/Command.h"
#include "PiSubmarine/Input/Fake/Axis.h"
#include "PiSubmarine/Input/Fake/Key.h"
#include "PiSubmarine/Operator/Station/Control/Controller.h"

namespace PiSubmarine::Operator::Station::Control
{
    namespace
    {
        class LoggerFactoryStub final : public Logging::Api::IFactory
        {
        public:
            [[nodiscard]] std::shared_ptr<spdlog::logger> CreateLogger(std::string_view name) override
            {
                return std::make_shared<spdlog::logger>(
                    std::string(name),
                    std::make_shared<spdlog::sinks::null_sink_mt>());
            }
        };

        struct SubmittedCommandCapture
        {
            std::optional<::PiSubmarine::Control::Api::Input::OperatorCommand> LastCommand;
        };
    }

    TEST(ControllerTest, LampAxisChangesIntensityIncrementally)
    {
        LoggerFactoryStub loggerFactory;
        testing::NiceMock<::PiSubmarine::Control::Api::Input::ISinkMock> sink;
        SubmittedCommandCapture submittedCommand;
        double lampAxisValue = 0.5;
        ::PiSubmarine::Input::Fake::Axis lampAxis([&lampAxisValue]()
        {
            return SignedNormalizedFraction(lampAxisValue);
        });
        Controller controller(sink, Controller::Config{}, loggerFactory);

        EXPECT_CALL(sink, Submit(testing::_))
            .WillRepeatedly(testing::Invoke([&submittedCommand](const auto& command)
            {
                submittedCommand.LastCommand = command;
                return Error::Api::Result<void>{};
            }));

        controller.SetLampAxis(&lampAxis);
        controller.Tick(std::chrono::seconds(1), std::chrono::seconds(2));

        ASSERT_TRUE(submittedCommand.LastCommand.has_value());
        ASSERT_TRUE(submittedCommand.LastCommand->LampIntensity.has_value());
        EXPECT_DOUBLE_EQ(static_cast<double>(submittedCommand.LastCommand->LampIntensity->Intensity()), 0.25);
    }

    TEST(ControllerTest, GimbalCenterKeyResetsPitchToZero)
    {
        LoggerFactoryStub loggerFactory;
        testing::NiceMock<::PiSubmarine::Control::Api::Input::ISinkMock> sink;
        SubmittedCommandCapture submittedCommand;
        double gimbalAxisValue = 1.0;
        ::PiSubmarine::Input::Api::KeyState gimbalCenterState = ::PiSubmarine::Input::Api::KeyState::Released;
        ::PiSubmarine::Input::Fake::Axis gimbalAxis([&gimbalAxisValue]()
        {
            return SignedNormalizedFraction(gimbalAxisValue);
        });
        ::PiSubmarine::Input::Fake::Key gimbalCenterKey([&gimbalCenterState]()
        {
            return gimbalCenterState;
        });
        Controller controller(sink, Controller::Config{}, loggerFactory);

        EXPECT_CALL(sink, Submit(testing::_))
            .WillRepeatedly(testing::Invoke([&submittedCommand](const auto& command)
            {
                submittedCommand.LastCommand = command;
                return Error::Api::Result<void>{};
            }));

        controller.SetGimbalPitchAxis(&gimbalAxis);
        controller.SetGimbalCenterKey(&gimbalCenterKey);
        controller.Tick(std::chrono::seconds(1), std::chrono::milliseconds(500));

        gimbalAxisValue = 0.0;
        gimbalCenterState = ::PiSubmarine::Input::Api::KeyState::Pressed;
        controller.Tick(std::chrono::seconds(2), std::chrono::milliseconds(10));

        ASSERT_TRUE(submittedCommand.LastCommand.has_value());
        ASSERT_TRUE(submittedCommand.LastCommand->GimbalTarget.has_value());
        EXPECT_DOUBLE_EQ(submittedCommand.LastCommand->GimbalTarget->Pitch().ToDegrees().Value, 0.0);
    }

    TEST(ControllerTest, BallastAxisChangesBallastPositionOnlyInBallastMode)
    {
        LoggerFactoryStub loggerFactory;
        testing::NiceMock<::PiSubmarine::Control::Api::Input::ISinkMock> sink;
        SubmittedCommandCapture submittedCommand;
        double ballastAxisValue = 1.0;
        ::PiSubmarine::Input::Fake::Axis ballastAxis([&ballastAxisValue]()
        {
            return SignedNormalizedFraction(ballastAxisValue);
        });
        Controller controller(sink, Controller::Config{}, loggerFactory);

        EXPECT_CALL(sink, Submit(testing::_))
            .WillRepeatedly(testing::Invoke([&submittedCommand](const auto& command)
            {
                submittedCommand.LastCommand = command;
                return Error::Api::Result<void>{};
            }));

        controller.SetBallastAxis(&ballastAxis);
        controller.Tick(std::chrono::seconds(1), std::chrono::seconds(1));

        controller.SetVerticalKeepCurrentMode();
        controller.Tick(std::chrono::seconds(2), std::chrono::seconds(2));

        ballastAxisValue = 0.0;
        controller.SetVerticalBallastPositionMode();
        controller.Tick(std::chrono::seconds(3), std::chrono::seconds(1));

        ASSERT_TRUE(submittedCommand.LastCommand.has_value());
        const auto* ballastPosition =
            submittedCommand.LastCommand->VerticalControl.TryGet<::PiSubmarine::Control::Vertical::Api::Command::SetBallastPosition>();
        ASSERT_NE(ballastPosition, nullptr);
        EXPECT_DOUBLE_EQ(static_cast<double>(ballastPosition->Position), 0.6);
    }

    TEST(ControllerTest, BallastAxisChangesDepthTargetInDepthMode)
    {
        LoggerFactoryStub loggerFactory;
        testing::NiceMock<::PiSubmarine::Control::Api::Input::ISinkMock> sink;
        SubmittedCommandCapture submittedCommand;
        double ballastAxisValue = 1.0;
        ::PiSubmarine::Input::Fake::Axis ballastAxis([&ballastAxisValue]()
        {
            return SignedNormalizedFraction(ballastAxisValue);
        });
        Controller controller(sink, Controller::Config{}, loggerFactory);

        EXPECT_CALL(sink, Submit(testing::_))
            .WillRepeatedly(testing::Invoke([&submittedCommand](const auto& command)
            {
                submittedCommand.LastCommand = command;
                return Error::Api::Result<void>{};
            }));

        controller.SetBallastAxis(&ballastAxis);
        controller.SetVerticalDepthTargetMode();
        controller.Tick(std::chrono::seconds(1), std::chrono::seconds(2));

        ASSERT_TRUE(submittedCommand.LastCommand.has_value());
        const auto* depthTarget =
            submittedCommand.LastCommand->VerticalControl.TryGet<::PiSubmarine::Control::Vertical::Api::Command::SetDepthTarget>();
        ASSERT_NE(depthTarget, nullptr);
        EXPECT_DOUBLE_EQ(depthTarget->Depth.Value, 0.5);
    }
}
