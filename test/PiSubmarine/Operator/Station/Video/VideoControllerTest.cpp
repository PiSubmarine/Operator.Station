#include <gtest/gtest.h>

#include <memory>

#include <spdlog/logger.h>
#include <spdlog/sinks/null_sink.h>

#include "PiSubmarine/Lease/Api/ILeaseIssuerMock.h"
#include "PiSubmarine/Operator/Station/Video/IPipeline.h"
#include "PiSubmarine/Operator/Station/Video/VideoController.h"
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

    TEST(VideoControllerTest, TickAcquiresLeaseAndSubscribesAfterStart)
    {
        LoggerFactoryStub loggerFactory;
        testing::StrictMock<Lease::Api::ILeaseIssuerMock> leaseIssuer;
        testing::StrictMock<::PiSubmarine::Video::Subscription::Api::IServiceMock> subscriptionService;
        auto pipelineBuilder = std::make_shared<FakePipelineBuilder>();
        VideoController controller(Config{}, loggerFactory, leaseIssuer, subscriptionService, pipelineBuilder);
        const auto leaseGrant = Lease::Api::LeaseGrant{
            .Lease = Lease::Api::Lease{
                .Id = Lease::Api::LeaseId{.Value = "lease-1"},
                .Resource = Lease::Api::ResourceId{.Value = "video-main"},
                .Duration = std::chrono::seconds(4)}};
        const auto subscribeRequest = ::PiSubmarine::Video::Subscription::Api::SubscribeRequest{
            .LeaseId = Lease::Api::LeaseId{.Value = "lease-1"},
            .ClientEndpoint = {.Host = "127.0.0.1", .Port = 5004}};

        EXPECT_CALL(leaseIssuer, AcquireLease(Lease::Api::LeaseRequest{
                        .Resource = Lease::Api::ResourceId{.Value = "video-main"}}))
            .WillOnce(testing::Return(Error::Api::Result<Lease::Api::LeaseGrant>(leaseGrant)));
        EXPECT_CALL(subscriptionService, Subscribe(subscribeRequest))
            .WillOnce(testing::Return(Error::Api::Result<void>{}));

        ASSERT_TRUE(controller.Start().has_value());
        controller.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));

        const auto statusResult = controller.GetStatus();
        ASSERT_TRUE(statusResult.has_value());
        EXPECT_TRUE(statusResult->HasLease);
        EXPECT_TRUE(statusResult->IsSubscribed);
        EXPECT_EQ(pipelineBuilder->BuildCount, 0);
    }

    TEST(VideoControllerTest, RebuildsPipelineWhenTailFactoryIsAvailable)
    {
        LoggerFactoryStub loggerFactory;
        testing::StrictMock<Lease::Api::ILeaseIssuerMock> leaseIssuer;
        testing::StrictMock<::PiSubmarine::Video::Subscription::Api::IServiceMock> subscriptionService;
        auto pipelineBuilder = std::make_shared<FakePipelineBuilder>();
        VideoController controller(
            Config{
                .ReceiveEndpoint = {.BindAddress = "0.0.0.0", .Port = 5600},
                .SubscriptionEndpoint = {.Host = "192.168.1.20", .Port = 5600}},
            loggerFactory,
            leaseIssuer,
            subscriptionService,
            pipelineBuilder);
        TailFactoryStub tailFactory;
        const auto leaseGrant = Lease::Api::LeaseGrant{
            .Lease = Lease::Api::Lease{
                .Id = Lease::Api::LeaseId{.Value = "lease-2"},
                .Resource = Lease::Api::ResourceId{.Value = "video-main"},
                .Duration = std::chrono::seconds(6)}};
        const auto subscribeRequest = ::PiSubmarine::Video::Subscription::Api::SubscribeRequest{
            .LeaseId = Lease::Api::LeaseId{.Value = "lease-2"},
            .ClientEndpoint = {.Host = "192.168.1.20", .Port = 5600}};

        EXPECT_CALL(leaseIssuer, AcquireLease(testing::_))
            .WillOnce(testing::Return(Error::Api::Result<Lease::Api::LeaseGrant>(leaseGrant)));
        EXPECT_CALL(subscriptionService, Subscribe(subscribeRequest))
            .WillOnce(testing::Return(Error::Api::Result<void>{}));

        ASSERT_TRUE(controller.SetPipelineTailFactory(tailFactory).has_value());
        ASSERT_TRUE(controller.Start().has_value());
        controller.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));

        EXPECT_EQ(pipelineBuilder->BuildCount, 1);
        EXPECT_EQ(pipelineBuilder->LastReceiveEndpoint, (ReceiveEndpoint{.BindAddress = "0.0.0.0", .Port = 5600}));
        EXPECT_EQ(pipelineBuilder->LastTailFactory, &tailFactory);
    }

    TEST(VideoControllerTest, RenewsLeaseHalfwayThroughLifetime)
    {
        LoggerFactoryStub loggerFactory;
        testing::StrictMock<Lease::Api::ILeaseIssuerMock> leaseIssuer;
        testing::StrictMock<::PiSubmarine::Video::Subscription::Api::IServiceMock> subscriptionService;
        auto pipelineBuilder = std::make_shared<FakePipelineBuilder>();
        VideoController controller(Config{}, loggerFactory, leaseIssuer, subscriptionService, pipelineBuilder);
        const auto leaseGrant = Lease::Api::LeaseGrant{
            .Lease = Lease::Api::Lease{
                .Id = Lease::Api::LeaseId{.Value = "lease-3"},
                .Resource = Lease::Api::ResourceId{.Value = "video-main"},
                .Duration = std::chrono::seconds(4)}};
        const auto renewedLease = Lease::Api::Lease{
            .Id = Lease::Api::LeaseId{.Value = "lease-3"},
            .Resource = Lease::Api::ResourceId{.Value = "video-main"},
            .Duration = std::chrono::seconds(4)};

        EXPECT_CALL(leaseIssuer, AcquireLease(testing::_))
            .WillOnce(testing::Return(Error::Api::Result<Lease::Api::LeaseGrant>(leaseGrant)));
        EXPECT_CALL(subscriptionService, Subscribe(testing::_))
            .WillOnce(testing::Return(Error::Api::Result<void>{}));
        EXPECT_CALL(leaseIssuer, RenewLease(Lease::Api::LeaseId{.Value = "lease-3"}))
            .WillOnce(testing::Return(Error::Api::Result<Lease::Api::Lease>(renewedLease)));

        ASSERT_TRUE(controller.Start().has_value());
        controller.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));
        controller.Tick(std::chrono::seconds(3), std::chrono::milliseconds(10));
    }

    TEST(VideoControllerTest, ReconfiguringReceiveEndpointRebuildsPipeline)
    {
        LoggerFactoryStub loggerFactory;
        testing::StrictMock<Lease::Api::ILeaseIssuerMock> leaseIssuer;
        testing::StrictMock<::PiSubmarine::Video::Subscription::Api::IServiceMock> subscriptionService;
        auto pipelineBuilder = std::make_shared<FakePipelineBuilder>();
        VideoController controller(Config{}, loggerFactory, leaseIssuer, subscriptionService, pipelineBuilder);
        TailFactoryStub tailFactory;
        const auto leaseGrant = Lease::Api::LeaseGrant{
            .Lease = Lease::Api::Lease{
                .Id = Lease::Api::LeaseId{.Value = "lease-4"},
                .Resource = Lease::Api::ResourceId{.Value = "video-main"},
                .Duration = std::chrono::seconds(6)}};

        EXPECT_CALL(leaseIssuer, AcquireLease(testing::_))
            .WillOnce(testing::Return(Error::Api::Result<Lease::Api::LeaseGrant>(leaseGrant)));
        EXPECT_CALL(subscriptionService, Subscribe(testing::_))
            .WillOnce(testing::Return(Error::Api::Result<void>{}));

        ASSERT_TRUE(controller.SetPipelineTailFactory(tailFactory).has_value());
        ASSERT_TRUE(controller.Start().has_value());
        controller.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));
        ASSERT_TRUE(controller.SetReceiveEndpoint({.BindAddress = "0.0.0.0", .Port = 5500}).has_value());
        controller.Tick(std::chrono::seconds(2), std::chrono::milliseconds(10));

        EXPECT_EQ(pipelineBuilder->BuildCount, 2);
        EXPECT_EQ(pipelineBuilder->LastReceiveEndpoint.Port, 5500);
    }
}
