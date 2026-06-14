#include "PiSubmarine/Operator/Station/Qt/StationApp.h"

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQmlApplicationEngine>
#include <QUrl>

#include <gst/gst.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"
#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Qt/QmlVideoSinkTailFactory.h"
#include "PiSubmarine/Operator/Station/Qt/VideoRuntimeWorker.h"
#include "PiSubmarine/Operator/Station/Video/Config.h"
#include "PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace PiSubmarine::Operator::Station::Video
{
    void RegisterStaticPlugins(const std::shared_ptr<spdlog::logger>& logger);
}

namespace PiSubmarine::Operator::Station::Qt
{
    namespace
    {
        class SpdlogFactory final : public Logging::Api::IFactory
        {
        public:
            [[nodiscard]] std::shared_ptr<spdlog::logger> CreateLogger(std::string_view name) override
            {
                const auto loggerName = std::string(name);
                if (auto existingLogger = spdlog::get(loggerName))
                {
                    return existingLogger;
                }

                static auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                static auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                    "PiSubmarine.Operator.Station.log",
                    true);
                const std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};

                auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
                logger->set_level(spdlog::level::info);
                logger->flush_on(spdlog::level::info);
                spdlog::register_logger(logger);
                return logger;
            }
        };

        class LocalLeaseIssuer final : public Lease::Api::ILeaseIssuer
        {
        public:
            [[nodiscard]] Error::Api::Result<Lease::Api::LeaseGrant> AcquireLease(
                const Lease::Api::LeaseRequest& request) override
            {
                ++m_Counter;
                return Lease::Api::LeaseGrant{
                    .Lease = Lease::Api::Lease{
                        .Id = Lease::Api::LeaseId{.Value = "local-video-lease-" + std::to_string(m_Counter)},
                        .Resource = request.Resource,
                        .Duration = std::chrono::seconds(4)}};
            }

            [[nodiscard]] Error::Api::Result<Lease::Api::Lease> RenewLease(
                const Lease::Api::LeaseId& leaseId) override
            {
                return Lease::Api::Lease{
                    .Id = leaseId,
                    .Resource = Lease::Api::ResourceId{.Value = "video-main"},
                    .Duration = std::chrono::seconds(4)};
            }

            [[nodiscard]] Error::Api::Result<void> ReleaseLease(const Lease::Api::LeaseId& leaseId) override
            {
                static_cast<void>(leaseId);
                return {};
            }

        private:
            int m_Counter = 0;
        };

        class LocalSubscriptionService final : public ::PiSubmarine::Video::Subscription::Api::IService
        {
        public:
            [[nodiscard]] Error::Api::Result<void> Subscribe(
                const ::PiSubmarine::Video::Subscription::Api::SubscribeRequest& request) override
            {
                m_LastRequest = request;
                return {};
            }

            [[nodiscard]] Error::Api::Result<void> Unsubscribe(
                const ::PiSubmarine::Video::Subscription::Api::UnsubscribeRequest& request) override
            {
                m_LastLeaseId = request.LeaseId;
                return {};
            }

        private:
            ::PiSubmarine::Video::Subscription::Api::SubscribeRequest m_LastRequest{};
            Lease::Api::LeaseId m_LastLeaseId{};
        };

        bool EnsureGstreamerReadyForQml(const std::shared_ptr<spdlog::logger>& logger)
        {
            GError* error = nullptr;
            if (!gst_is_initialized() && !gst_init_check(nullptr, nullptr, &error))
            {
                if (logger && error != nullptr)
                {
                    logger->error("gst_init_check failed while preparing QML video item: {}", error->message);
                }

                if (error != nullptr)
                {
                    g_error_free(error);
                }

                return false;
            }

            Video::RegisterStaticPlugins(logger);

#if defined(_WIN32)
            if (auto* warmupSink = gst_element_factory_make("qml6d3d11sink", nullptr))
            {
                gst_object_unref(GST_OBJECT(warmupSink));
            }
            else if (logger)
            {
                logger->error("Failed to warm up qml6d3d11sink before QML load");
                return false;
            }
#endif

            return true;
        }
    }

    bool StationApp::ConfigureCommandLine(QGuiApplication& application, QCommandLineParser& parser) const
    {
        parser.setApplicationDescription("PiSubmarine Operator Station");
        parser.addHelpOption();
        parser.addOption(QCommandLineOption("fake-video", "Use fake test-pattern video instead of RTP."));
        parser.addOption(QCommandLineOption("video-bind-address", "Local RTP bind address.", "address", "0.0.0.0"));
        parser.addOption(QCommandLineOption("video-port", "Local RTP bind port.", "port", "5004"));
        parser.process(application);
        return true;
    }

    bool StationApp::LoadMainWindow()
    {
        if (!EnsureGstreamerReadyForQml(m_Logger))
        {
            return false;
        }

        const QUrl mainWindowUrl(
#if defined(_WIN32)
            "qrc:/PiSubmarine/Operator/Station/qml/Main.Windows.qml"
#else
            "qrc:/PiSubmarine/Operator/Station/qml/Main.qml"
#endif
        );
        m_Engine.load(mainWindowUrl);
        if (m_Engine.rootObjects().isEmpty())
        {
            return false;
        }

        m_VideoItem = m_Engine.rootObjects().front()->findChild<QQuickItem*>("videoSurface");
        return m_VideoItem != nullptr;
    }

    bool StationApp::StartVideoRuntime(bool useFakeVideo, std::uint16_t videoPort, const std::string& bindAddress)
    {
        if (m_VideoItem == nullptr)
        {
            return false;
        }

        static SpdlogFactory loggerFactory;
        static LocalLeaseIssuer leaseIssuer;
        static LocalSubscriptionService subscriptionService;
        m_Logger = loggerFactory.CreateLogger("Operator.Station.App");

        Video::Config videoConfig;
        videoConfig.ReceiveEndpoint.BindAddress = bindAddress;
        videoConfig.ReceiveEndpoint.Port = videoPort;
        videoConfig.SubscriptionEndpoint = {.Host = "127.0.0.1", .Port = videoPort};

        std::shared_ptr<Video::IPipelineBuilder> pipelineBuilder =
            useFakeVideo
                ? Video::CreateFakePipelineBuilder(loggerFactory)
                : Video::CreateRtpPipelineBuilder(loggerFactory);

        m_TailFactory = std::make_unique<QmlVideoSinkTailFactory>(*m_VideoItem, m_Logger);

        m_RuntimeWorker = new VideoRuntimeWorker(
            videoConfig,
            loggerFactory,
            leaseIssuer,
            subscriptionService,
            std::move(pipelineBuilder),
            *m_TailFactory);
        m_RuntimeWorker->moveToThread(&m_RuntimeThread);
        QObject::connect(&m_RuntimeThread, &QThread::started, m_RuntimeWorker, &VideoRuntimeWorker::Start);
        QObject::connect(&m_RuntimeThread, &QThread::finished, m_RuntimeWorker, &QObject::deleteLater);
        m_RuntimeThread.start();
        if (m_Logger)
        {
            m_Logger->info(
                "Video runtime started with {} pipeline on {}:{}",
                useFakeVideo ? "fake" : "rtp",
                bindAddress,
                videoPort);
        }
        return true;
    }

    void StationApp::StopVideoRuntime()
    {
        if (m_RuntimeWorker != nullptr)
        {
            QMetaObject::invokeMethod(m_RuntimeWorker, &VideoRuntimeWorker::Stop, ::Qt::BlockingQueuedConnection);
            m_RuntimeWorker = nullptr;
        }

        if (m_RuntimeThread.isRunning())
        {
            m_RuntimeThread.quit();
            m_RuntimeThread.wait();
        }

        m_TailFactory.reset();

        if (m_Logger)
        {
            m_Logger->info("Video runtime stopped");
        }
    }

    int StationApp::Run(QGuiApplication& application)
    {
        QCommandLineParser parser;
        ConfigureCommandLine(application, parser);

        static SpdlogFactory loggerFactory;
        m_Logger = loggerFactory.CreateLogger("Operator.Station.App");

        if (!LoadMainWindow())
        {
            return 1;
        }

        const auto bindAddress = parser.value("video-bind-address").toStdString();
        const auto videoPort = static_cast<std::uint16_t>(parser.value("video-port").toUShort());
        if (!StartVideoRuntime(parser.isSet("fake-video"), videoPort, bindAddress))
        {
            return 1;
        }

        QObject::connect(&application, &QGuiApplication::aboutToQuit, &application, [this]()
        {
            StopVideoRuntime();
        });

        return application.exec();
    }
}
