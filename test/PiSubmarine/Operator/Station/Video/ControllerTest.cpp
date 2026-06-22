#include <gtest/gtest.h>

#include <memory>

#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>

#include "PiSubmarine/Lease/Api/ILeaseIssuerMock.h"
#include "PiSubmarine/Operator/Station/Video/Controller.h"
#include "PiSubmarine/Operator/Station/Video/IPipeline.h"
#include "PiSubmarine/Video/Subscription/Api/IServiceMock.h"

namespace PiSubmarine::Operator::Station::Video
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

        class TailFactoryStub final : public IVideoPipelineTailFactory
        {
        public:
            [[nodiscard]] Error::Api::Result<GstElement*> CreatePipelineTail() override
            {
                return nullptr;
            }
        };

        class FakePipeline final : public IPipeline
        {
        public:
            Error::Api::Result<void> Stop() override
            {
                Running = false;
                ++StopCount;
                return {};
            }

            void PollBus() override
            {
                ++PollCount;
            }

            [[nodiscard]] bool IsRunning() const noexcept override
            {
                return Running;
            }

            bool Running = true;
            int StopCount = 0;
            int PollCount = 0;
        };

        class FakePipelineBuilder final : public IPipelineBuilder
        {
        public:
            [[nodiscard]] std::unique_ptr<IPipeline> Build(
                const ReceiveEndpoint& receiveEndpoint,
                IVideoPipelineTailFactory& tailFactory) override
            {
                LastReceiveEndpoint = receiveEndpoint;
                LastTailFactory = &tailFactory;
                ++BuildCount;

                auto pipeline = std::make_unique<FakePipeline>();
                LastBuiltPipeline = pipeline.get();
                return pipeline;
            }

            ReceiveEndpoint LastReceiveEndpoint{};
            IVideoPipelineTailFactory* LastTailFactory = nullptr;
            FakePipeline* LastBuiltPipeline = nullptr;
            int BuildCount = 0;
        };
    }

    TEST(ControllerTest, TickAcquiresLeaseAndSubscribesAfterStart)
    {
        LoggerFactoryStub loggerFactory;
        testing::NiceMock<::PiSubmarine::Lease::Api::ILeaseIssuerMock> leaseIssuer;
        testing::NiceMock<::PiSubmarine::Video::Subscription::Api::IServiceMock> subscriptionService;
        auto pipelineBuilder = std::make_shared<FakePipelineBuilder>();
        TailFactoryStub tailFactory;
        Controller controller(Config{}, loggerFactory, leaseIssuer, subscriptionService, pipelineBuilder, tailFactory);
        const auto leaseGrant = ::PiSubmarine::Lease::Api::LeaseGrant{
            .GrantedLease = ::PiSubmarine::Lease::Api::Lease{
                .Id = ::PiSubmarine::Lease::Api::LeaseId{.Value = "lease-1"},
                .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "video-main"},
                .Duration = std::chrono::seconds(4)}};
        const auto subscribeRequest = ::PiSubmarine::Video::Subscription::Api::SubscribeRequest{
            .LeaseId = ::PiSubmarine::Lease::Api::LeaseId{.Value = "lease-1"},
            .ClientEndpoint = {.Host = "127.0.0.1", .Port = 5004}};

        EXPECT_CALL(leaseIssuer, AcquireLease(::PiSubmarine::Lease::Api::LeaseRequest{
                        .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "video-main"}}))
            .WillOnce(testing::Return(Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>(leaseGrant)));
        EXPECT_CALL(subscriptionService, Subscribe(subscribeRequest))
            .WillOnce(testing::Return(Error::Api::Result<void>{}));
        EXPECT_CALL(subscriptionService, Unsubscribe(testing::_))
            .WillRepeatedly(testing::Return(Error::Api::Result<void>{}));
        EXPECT_CALL(leaseIssuer, ReleaseLease(testing::_))
            .WillRepeatedly(testing::Return(Error::Api::Result<void>{}));

        controller.Start();
        controller.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));

        const auto statusResult = controller.GetStatus();
        ASSERT_TRUE(statusResult.has_value());
        EXPECT_TRUE(statusResult->HasLease);
        EXPECT_TRUE(statusResult->IsSubscribed);
        EXPECT_EQ(pipelineBuilder->BuildCount, 1);
    }

    TEST(ControllerTest, EndpointChangesMarkPipelineForRebuild)
    {
        LoggerFactoryStub loggerFactory;
        testing::NiceMock<::PiSubmarine::Lease::Api::ILeaseIssuerMock> leaseIssuer;
        testing::NiceMock<::PiSubmarine::Video::Subscription::Api::IServiceMock> subscriptionService;
        auto pipelineBuilder = std::make_shared<FakePipelineBuilder>();
        TailFactoryStub tailFactory;
        Controller controller(Config{}, loggerFactory, leaseIssuer, subscriptionService, pipelineBuilder, tailFactory);
        const auto leaseGrant = ::PiSubmarine::Lease::Api::LeaseGrant{
            .GrantedLease = ::PiSubmarine::Lease::Api::Lease{
                .Id = ::PiSubmarine::Lease::Api::LeaseId{.Value = "lease-2"},
                .Resource = ::PiSubmarine::Lease::Api::ResourceId{.Value = "video-main"},
                .Duration = std::chrono::seconds(4)}};

        EXPECT_CALL(leaseIssuer, AcquireLease(testing::_))
            .WillOnce(testing::Return(Error::Api::Result<::PiSubmarine::Lease::Api::LeaseGrant>(leaseGrant)));
        EXPECT_CALL(subscriptionService, Subscribe(testing::_))
            .WillOnce(testing::Return(Error::Api::Result<void>{}));
        EXPECT_CALL(subscriptionService, Unsubscribe(testing::_))
            .WillRepeatedly(testing::Return(Error::Api::Result<void>{}));
        EXPECT_CALL(leaseIssuer, ReleaseLease(testing::_))
            .WillRepeatedly(testing::Return(Error::Api::Result<void>{}));

        controller.Start();
        controller.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));
        controller.SetReceiveEndpoint("0.0.0.0", 5600);
        controller.Tick(std::chrono::seconds(2), std::chrono::milliseconds(10));

        EXPECT_EQ(pipelineBuilder->BuildCount, 2);
        EXPECT_EQ(pipelineBuilder->LastReceiveEndpoint.Port, 5600);
    }
}
