#include <memory>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QThread>
#include <QUrl>

#include <gst/gst.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Operator/Station/Infrastructure/QtLog.h"
#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Gstreamer/Build/Plugins.h"
#include "PiSubmarine/Operator/Station/Infrastructure/SpdlogFactory.h"
#include "PiSubmarine/Operator/Station/Input/Controller.h"
#include "PiSubmarine/Operator/Station/Input/FakeSink.h"
#include "PiSubmarine/Operator/Station/Input/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Lease/FakeIssuer.h"
#include "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h"
#include "PiSubmarine/Operator/Station/Lease/ThreadWorker.h"
#include "PiSubmarine/Operator/Station/Telemetry/FakeController.h"
#include "PiSubmarine/Operator/Station/Telemetry/FakeSubscriptionService.h"
#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/Controller.h"
#include "PiSubmarine/Operator/Station/Video/FakeController.h"
#include "PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h"
#include "PiSubmarine/Operator/Station/Video/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/View/VideoSurfaceItem.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace
{
    class LocalVideoSubscriptionService final : public ::PiSubmarine::Video::Subscription::Api::IService
    {
    public:
        [[nodiscard]] ::PiSubmarine::Error::Api::Result<void> Subscribe(
            const ::PiSubmarine::Video::Subscription::Api::SubscribeRequest& request) override
        {
            m_LastRequest = request;
            return {};
        }

        [[nodiscard]] ::PiSubmarine::Error::Api::Result<void> Unsubscribe(
            const ::PiSubmarine::Video::Subscription::Api::UnsubscribeRequest& request) override
        {
            m_LastLeaseId = request.LeaseId;
            return {};
        }

    private:
        ::PiSubmarine::Video::Subscription::Api::SubscribeRequest m_LastRequest{};
        ::PiSubmarine::Lease::Api::LeaseId m_LastLeaseId{};
    };

    // TODO Move to GstreamerPipeline. Rename to EnsureGstreamerInitialized
    bool EnsureGstreamerReadyForQml(const std::shared_ptr<spdlog::logger>& logger)
    {
        GError* error = nullptr;
        if (!gst_is_initialized() && !gst_init_check(nullptr, nullptr, &error))
        {
            if (logger && error != nullptr)
            {
                logger->error("gst_init_check failed: {}", error->message);
            }
            if (error != nullptr)
            {
                g_error_free(error);
            }
            return false;
        }

        ::PiSubmarine::Gstreamer::Build::Plugins::RegisterStatic(logger);
        return true;
    }
}

int main(int argc, char* argv[])
{
    PiSubmarine::Operator::Station::Infrastructure::SpdlogFactory loggerFactory;
    auto qtLogger = loggerFactory.CreateLogger("Qt");
    PiSubmarine::Operator::Station::Infrastructure::InstallQtMessageHandler(qtLogger);

    Q_INIT_RESOURCE(qml);

    QGuiApplication application(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("fake-video", "Use a local fake test-pattern video pipeline."));
    parser.addOption(QCommandLineOption("video-bind-address", "Local RTP bind address.", "address", "0.0.0.0"));
    parser.addOption(QCommandLineOption("video-port", "Local RTP bind port.", "port", "5004"));
    parser.addOption(QCommandLineOption("telemetry-port", "Local telemetry subscription port.", "port", "6100"));
    parser.process(application);


    const auto logger = loggerFactory.CreateLogger("Operator.Station.Main");
    if (!logger || !EnsureGstreamerReadyForQml(logger))
    {
        return 1;
    }

    PiSubmarine::Operator::Station::Video::View::ViewModel videoViewModel;
    videoViewModel.SetReceiveBindAddress(parser.value("video-bind-address"));
    videoViewModel.SetReceivePort(static_cast<quint16>(parser.value("video-port").toUShort()));
    videoViewModel.SetSubscriptionHost("127.0.0.1");
    videoViewModel.SetSubscriptionPort(static_cast<quint16>(parser.value("video-port").toUShort()));

    PiSubmarine::Operator::Station::Input::View::ViewModel inputViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Lamp::ViewModel lampTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel motorTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Battery::ViewModel batteryTelemetryViewModel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("videoViewModel", &videoViewModel);
    engine.rootContext()->setContextProperty("inputViewModel", &inputViewModel);
    engine.rootContext()->setContextProperty("lampTelemetryViewModel", &lampTelemetryViewModel);
    engine.rootContext()->setContextProperty("motorTelemetryViewModel", &motorTelemetryViewModel);
    engine.rootContext()->setContextProperty("batteryTelemetryViewModel", &batteryTelemetryViewModel);
    qmlRegisterType<PiSubmarine::Operator::Station::Video::View::VideoSurfaceItem>(
        "PiSubmarine.Operator.Station",
        1,
        0,
        "VideoSurfaceItem");

    const QUrl mainWindowUrl(
#if defined(_WIN32)
        "qrc:/PiSubmarine/Operator/Station/qml/Main.Windows.qml"
#else
        "qrc:/PiSubmarine/Operator/Station/qml/Main.qml"
#endif
    );
    engine.load(mainWindowUrl);
    if (engine.rootObjects().isEmpty())
    {
        // TODO Add error log line
        return 1;
    }

    auto* videoItem = engine.rootObjects().front()->findChild<QQuickItem*>("videoSurface");
    if (videoItem == nullptr)
    {
        // TODO Add error log line
        return 1;
    }

    QThread controllerThread;
    QThread leaseThread;

    PiSubmarine::Operator::Station::Lease::FakeIssuer blockingLeaseIssuer;
    auto* leaseWorker = new PiSubmarine::Operator::Station::Lease::ThreadWorker(blockingLeaseIssuer, loggerFactory);
    leaseWorker->moveToThread(&leaseThread);
    QObject::connect(&leaseThread, &QThread::finished, leaseWorker, &QObject::deleteLater);

    PiSubmarine::Operator::Station::Lease::SyncLeaseIssuerProxy leaseProxy(*leaseWorker);
    LocalVideoSubscriptionService videoSubscriptionService;
    PiSubmarine::Operator::Station::Telemetry::FakeSubscriptionService telemetrySubscriptionService;
    PiSubmarine::Operator::Station::Input::FakeSink fakeInputSink;
    PiSubmarine::Operator::Station::Video::View::QmlVideoSinkTailFactory videoTailFactory(*videoItem, logger);

    PiSubmarine::Operator::Station::Video::Config videoConfig;
    videoConfig.ReceiveEndpoint = {
        .BindAddress = videoViewModel.GetReceiveBindAddress().toStdString(),
        .Port = videoViewModel.GetReceivePort()};
    videoConfig.SubscriptionEndpoint = {
        .Host = videoViewModel.GetSubscriptionHost().toStdString(),
        .Port = videoViewModel.GetSubscriptionPort()};

    std::unique_ptr<PiSubmarine::Operator::Station::Video::Controller> videoController;
    if (parser.isSet("fake-video"))
    {
        videoController = PiSubmarine::Operator::Station::Video::CreateFakeController(
            videoConfig,
            loggerFactory,
            leaseProxy,
            videoSubscriptionService,
            videoTailFactory);
    }
    else
    {
        videoController = std::make_unique<PiSubmarine::Operator::Station::Video::Controller>(
            videoConfig,
            loggerFactory,
            leaseProxy,
            videoSubscriptionService,
            PiSubmarine::Operator::Station::Video::CreateRtpPipelineBuilder(loggerFactory),
            videoTailFactory);
    }

    auto telemetryParts = PiSubmarine::Operator::Station::Telemetry::CreateFakeController(
        leaseProxy,
        telemetrySubscriptionService,
        loggerFactory);
    auto inputController = std::make_unique<PiSubmarine::Operator::Station::Input::Controller>(
        fakeInputSink,
        inputViewModel,
        loggerFactory);

    QObject::connect(
        &videoViewModel,
        &PiSubmarine::Operator::Station::Video::View::ViewModel::ReceiveEndpointChanged,
        videoController.get(),
        &PiSubmarine::Operator::Station::Video::Controller::SetReceiveEndpoint,
        Qt::QueuedConnection);
    QObject::connect(
        &videoViewModel,
        &PiSubmarine::Operator::Station::Video::View::ViewModel::SubscriptionEndpointChanged,
        videoController.get(),
        &PiSubmarine::Operator::Station::Video::Controller::SetSubscriptionEndpoint,
        Qt::QueuedConnection);
    QObject::connect(
        telemetryParts.Lamp.get(),
        &PiSubmarine::Operator::Station::Telemetry::LampController::SnapshotChanged,
        &lampTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Lamp::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        telemetryParts.Motor.get(),
        &PiSubmarine::Operator::Station::Telemetry::MotorController::SnapshotChanged,
        &motorTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        telemetryParts.Battery.get(),
        &PiSubmarine::Operator::Station::Telemetry::BatteryController::SnapshotChanged,
        &batteryTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Battery::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        &inputViewModel,
        &PiSubmarine::Operator::Station::Input::View::ViewModel::IntentChanged,
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::SubmitCurrentIntent,
        Qt::QueuedConnection);

    videoController->moveToThread(&controllerThread);
    telemetryParts.Lamp->moveToThread(&controllerThread);
    telemetryParts.Motor->moveToThread(&controllerThread);
    telemetryParts.Battery->moveToThread(&controllerThread);
    telemetryParts.Coordinator->moveToThread(&controllerThread);
    inputController->moveToThread(&controllerThread);

    QObject::connect(&controllerThread, &QThread::started, videoController.get(), &PiSubmarine::Operator::Station::Video::Controller::Start);
    QObject::connect(&controllerThread, &QThread::started, telemetryParts.Coordinator.get(), &PiSubmarine::Operator::Station::Telemetry::Controller::Start);
    leaseThread.start();
    controllerThread.start();

    QObject::connect(&application, &QGuiApplication::aboutToQuit, &application, [&]()
    {
        QMetaObject::invokeMethod(videoController.get(), &PiSubmarine::Operator::Station::Video::Controller::Stop, Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(telemetryParts.Coordinator.get(), &PiSubmarine::Operator::Station::Telemetry::Controller::Stop, Qt::BlockingQueuedConnection);
        controllerThread.quit();
        controllerThread.wait();
        leaseThread.quit();
        leaseThread.wait();
    });

    return application.exec();
}
