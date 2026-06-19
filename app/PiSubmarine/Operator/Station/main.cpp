#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QThread>
#include <QUrl>
#include <QVariant>

#include <gst/gst.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Gstreamer/Build/Plugins.h"
#include "PiSubmarine/Operator/Station/Logging/QtLog.h"
#include "PiSubmarine/Operator/Station/Logging/SpdlogFactory.h"
#include "PiSubmarine/Operator/Station/Input/Controller.h"
#include "PiSubmarine/Operator/Station/Input/FakeSink.h"
#include "PiSubmarine/Operator/Station/Input/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Lease/FakeIssuer.h"
#include "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h"
#include "PiSubmarine/Operator/Station/Lease/ThreadWorker.h"
#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/FakeProviders.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"
#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/Controller.h"
#include "PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"
#include "PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h"
#include "PiSubmarine/Operator/Station/Video/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/View/VideoSurfaceItem.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace
{
    [[nodiscard]] std::optional<spdlog::level::level_enum> ParseLogLevel(const QString& value)
    {
        const auto normalizedValue = value.trimmed().toLower();

        if (normalizedValue == "trace")
        {
            return spdlog::level::trace;
        }
        if (normalizedValue == "debug")
        {
            return spdlog::level::debug;
        }
        if (normalizedValue == "info")
        {
            return spdlog::level::info;
        }
        if (normalizedValue == "warn")
        {
            return spdlog::level::warn;
        }
        if (normalizedValue == "error")
        {
            return spdlog::level::err;
        }
        if (normalizedValue == "critical")
        {
            return spdlog::level::critical;
        }
        if (normalizedValue == "off")
        {
            return spdlog::level::off;
        }

        return std::nullopt;
    }

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

    template <typename TObject>
    void DeleteLaterOnObjectThread(std::unique_ptr<TObject>& object)
    {
        if (!object)
        {
            return;
        }

        auto* objectThread = object->thread();
        if (objectThread == nullptr || QThread::currentThread() == objectThread)
        {
            object->deleteLater();
            object.release();
            return;
        }

        QMetaObject::invokeMethod(object.get(), &QObject::deleteLater, Qt::BlockingQueuedConnection);
        object.release();
    }
}

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(qml);

    QGuiApplication application(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("fake-video", "Use a local fake test-pattern video pipeline."));
    parser.addOption(QCommandLineOption(
        "log-level",
        "Set application log level: trace, debug, info, warn, error, critical, off.",
        "level",
        "info"));
    parser.addOption(QCommandLineOption("video-bind-address", "Local RTP bind address.", "address", "0.0.0.0"));
    parser.addOption(QCommandLineOption("video-port", "Local RTP bind port.", "port", "5004"));
    parser.addOption(QCommandLineOption("telemetry-port", "Local telemetry subscription port.", "port", "6100"));
    parser.process(application);

    const auto logLevel = ParseLogLevel(parser.value("log-level"));
    if (!logLevel.has_value())
    {
        qCritical("Invalid --log-level value. Use one of: trace, debug, info, warn, error, critical, off.");
        return 1;
    }

    PiSubmarine::Operator::Station::Logging::SpdlogFactory loggerFactory(*logLevel);
    auto qtLogger = loggerFactory.CreateLogger("Qt");
    PiSubmarine::Operator::Station::Logging::InstallQtMessageHandler(qtLogger);


    const auto logger = loggerFactory.CreateLogger("Operator.Station.Main");
    if (!logger || !PiSubmarine::Operator::Station::Video::GstreamerPipeline::EnsureGstreamerInitialized(logger))
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
    PiSubmarine::Operator::Station::Telemetry::View::Battery::ViewModel batteryTelemetryViewModel;

    constexpr std::size_t TelemetryMotorCount = 4;
    auto fakeTelemetryProviders = PiSubmarine::Operator::Station::Telemetry::CreateFakeProviders(TelemetryMotorCount);

    std::vector<std::unique_ptr<PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel>> motorTelemetryViewModels;
    std::vector<std::unique_ptr<PiSubmarine::Operator::Station::Telemetry::MotorController>> motorTelemetryControllers;
    std::vector<std::reference_wrapper<PiSubmarine::Operator::Station::Telemetry::MotorController>> motorTelemetryControllerRefs;
    QVariantList motorTelemetryViewModelList;

    motorTelemetryViewModels.reserve(TelemetryMotorCount);
    motorTelemetryControllers.reserve(TelemetryMotorCount);
    motorTelemetryControllerRefs.reserve(TelemetryMotorCount);
    motorTelemetryViewModelList.reserve(static_cast<qsizetype>(TelemetryMotorCount));

    for (std::size_t index = 0; index < TelemetryMotorCount; ++index)
    {
        motorTelemetryViewModels.push_back(
            std::make_unique<PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel>());
        motorTelemetryControllers.push_back(
            std::make_unique<PiSubmarine::Operator::Station::Telemetry::MotorController>(*fakeTelemetryProviders.Motors.at(index)));
        motorTelemetryControllerRefs.emplace_back(*motorTelemetryControllers.back());
        motorTelemetryViewModelList.push_back(QVariant::fromValue(static_cast<QObject*>(motorTelemetryViewModels.back().get())));
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("videoViewModel", &videoViewModel);
    engine.rootContext()->setContextProperty("inputViewModel", &inputViewModel);
    engine.rootContext()->setContextProperty("lampTelemetryViewModel", &lampTelemetryViewModel);
    engine.rootContext()->setContextProperty("motorTelemetryViewModels", motorTelemetryViewModelList);
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
        logger->error("Failed to load main QML window");
        return 1;
    }

    auto* videoItem = engine.rootObjects().front()->findChild<QQuickItem*>("videoSurface");
    if (videoItem == nullptr)
    {
        logger->error("Failed to locate QML video surface item");
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
    PiSubmarine::Operator::Station::Input::FakeSink fakeInputSink;
    PiSubmarine::Operator::Station::Video::View::QmlVideoSinkTailFactory videoTailFactory(*videoItem, logger);

    PiSubmarine::Operator::Station::Video::Config videoConfig;
    videoConfig.ReceiveEndpoint = {
        .BindAddress = videoViewModel.GetReceiveBindAddress().toStdString(),
        .Port = videoViewModel.GetReceivePort()};
    videoConfig.SubscriptionEndpoint = {
        .Host = videoViewModel.GetSubscriptionHost().toStdString(),
        .Port = videoViewModel.GetSubscriptionPort()};

    const auto videoPipelineBuilder = parser.isSet("fake-video")
        ? PiSubmarine::Operator::Station::Video::CreateFakePipelineBuilder(loggerFactory, videoTailFactory)
        : PiSubmarine::Operator::Station::Video::CreateRtpPipelineBuilder(loggerFactory, videoTailFactory);
    auto videoController = std::make_unique<PiSubmarine::Operator::Station::Video::Controller>(
        videoConfig,
        loggerFactory,
        leaseProxy,
        videoSubscriptionService,
        videoPipelineBuilder);
    auto lampTelemetryController = std::make_unique<PiSubmarine::Operator::Station::Telemetry::LampController>(*fakeTelemetryProviders.Lamp);
    auto batteryTelemetryController = std::make_unique<PiSubmarine::Operator::Station::Telemetry::BatteryController>(*fakeTelemetryProviders.Battery);
    auto telemetryController = std::make_unique<PiSubmarine::Operator::Station::Telemetry::Controller>(
        leaseProxy,
        *lampTelemetryController,
        std::move(motorTelemetryControllerRefs),
        *batteryTelemetryController,
        loggerFactory);
    auto inputController = std::make_unique<PiSubmarine::Operator::Station::Input::Controller>(
        fakeInputSink,
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
        lampTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::LampController::SnapshotChanged,
        &lampTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Lamp::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    for (std::size_t index = 0; index < motorTelemetryControllers.size(); ++index)
    {
        QObject::connect(
            motorTelemetryControllers.at(index).get(),
            &PiSubmarine::Operator::Station::Telemetry::MotorController::SnapshotChanged,
            motorTelemetryViewModels.at(index).get(),
            &PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel::SetSnapshot,
            Qt::QueuedConnection);
    }
    QObject::connect(
        batteryTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::BatteryController::SnapshotChanged,
        &batteryTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Battery::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        &inputViewModel,
        &PiSubmarine::Operator::Station::Input::View::ViewModel::IntentUpdated,
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::SubmitIntent,
        Qt::QueuedConnection);

    videoController->moveToThread(&controllerThread);
    lampTelemetryController->moveToThread(&controllerThread);
    for (const auto& motorTelemetryController : motorTelemetryControllers)
    {
        motorTelemetryController->moveToThread(&controllerThread);
    }
    batteryTelemetryController->moveToThread(&controllerThread);
    telemetryController->moveToThread(&controllerThread);
    inputController->moveToThread(&controllerThread);

    QObject::connect(&controllerThread, &QThread::started, videoController.get(), &PiSubmarine::Operator::Station::Video::Controller::Start);
    QObject::connect(&controllerThread, &QThread::started, telemetryController.get(), &PiSubmarine::Operator::Station::Telemetry::Controller::Start);
    leaseThread.start();
    controllerThread.start();

    QObject::connect(&application, &QGuiApplication::aboutToQuit, &application, [&]()
    {
        // TODO Controllers' destruction flow looks inconsistent between each other.
        QMetaObject::invokeMethod(videoController.get(), &PiSubmarine::Operator::Station::Video::Controller::Stop, Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(telemetryController.get(), &PiSubmarine::Operator::Station::Telemetry::Controller::Stop, Qt::BlockingQueuedConnection);

        DeleteLaterOnObjectThread(videoController);
        DeleteLaterOnObjectThread(lampTelemetryController);
        for (auto& motorTelemetryController : motorTelemetryControllers)
        {
            DeleteLaterOnObjectThread(motorTelemetryController);
        }
        DeleteLaterOnObjectThread(batteryTelemetryController);
        DeleteLaterOnObjectThread(telemetryController);
        DeleteLaterOnObjectThread(inputController);

        controllerThread.quit();
        controllerThread.wait();
        leaseThread.quit();
        leaseThread.wait();
    });

    return application.exec();
}
