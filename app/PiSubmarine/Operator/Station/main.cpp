#include <array>
#include <chrono>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQuickItem>
#include <QRegularExpression>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>
#include <QUrl>
#include <QVariant>

#include <gst/gst.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Operator/Station/Composition/FakeTelemetry.h"
#include "PiSubmarine/Operator/Station/Composition/ITelemetry.h"
#include "PiSubmarine/Operator/Station/Composition/RemoteTelemetry.h"
#include "PiSubmarine/Operator/Station/Logging/QtLog.h"
#include "PiSubmarine/Operator/Station/Logging/SpdlogFactory.h"
#include "PiSubmarine/Operator/Station/Input/Controller.h"
#include "PiSubmarine/Operator/Station/Input/FakeSink.h"
#include "PiSubmarine/Operator/Station/Input/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Lease/FakeIssuer.h"
#include "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h"
#include "PiSubmarine/Operator/Station/Lease/ThreadWorker.h"
#include "PiSubmarine/Operator/Station/Telemetry/BallastController.h"
#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/DepthController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"
#include "PiSubmarine/Operator/Station/Telemetry/ProximityController.h"
#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"
#include "PiSubmarine/Operator/Station/Telemetry/VideoStatusController.h"
#include "PiSubmarine/Operator/Station/Time/TickRunner.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Ballast/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Depth/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Proximity/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Video/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/Controller.h"
#include "PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"
#include "PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"
#include "PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h"
#include "PiSubmarine/Operator/Station/Video/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/View/VideoSurfaceItem.h"
#include "PiSubmarine/Udp/Api/Endpoint.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace
{
    [[nodiscard]] std::optional<PiSubmarine::Udp::Api::Endpoint> ParseEndpoint(const QString& value)
    {
        const auto trimmedValue = value.trimmed();
        const auto separator = trimmedValue.lastIndexOf(':');
        if (separator <= 0 || separator == trimmedValue.size() - 1)
        {
            return std::nullopt;
        }

        bool ok = false;
        const auto port = trimmedValue.sliced(separator + 1).toUInt(&ok);
        if (!ok || port > std::numeric_limits<std::uint16_t>::max())
        {
            return std::nullopt;
        }

        return PiSubmarine::Udp::Api::Endpoint{
            .Address = trimmedValue.first(separator).toStdString(),
            .Port = static_cast<std::uint16_t>(port)};
    }

    constexpr std::size_t TelemetryReceiveQueueCapacity = 16;
    constexpr std::size_t DefaultTelemetryMotorCount = 4;

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

    [[nodiscard]] std::optional<std::chrono::nanoseconds> ParseDuration(const QString& value)
    {
        static const QRegularExpression Pattern(R"(^\s*(\d+)\s*(ns|us|ms|s)\s*$)");

        const auto match = Pattern.match(value.trimmed().toLower());
        if (!match.hasMatch())
        {
            return std::nullopt;
        }

        bool ok = false;
        const auto count = match.captured(1).toULongLong(&ok);
        if (!ok || count == 0)
        {
            return std::nullopt;
        }

        const auto unit = match.captured(2);
        if (unit == "ns")
        {
            return std::chrono::nanoseconds(count);
        }
        if (unit == "us")
        {
            return std::chrono::microseconds(count);
        }
        if (unit == "ms")
        {
            return std::chrono::milliseconds(count);
        }
        if (unit == "s")
        {
            return std::chrono::seconds(count);
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
    parser.addOption(QCommandLineOption("fake-telemetry", "Use local fake telemetry providers."));
    parser.addOption(QCommandLineOption("fake-video", "Use a local fake test-pattern video pipeline."));
    parser.addOption(QCommandLineOption(
        "log-level",
        "Set application log level: trace, debug, info, warn, error, critical, off.",
        "level",
        "info"));
    parser.addOption(QCommandLineOption(
        "tickrate",
        "Set controllers thread tick period. Supported units: ns, us, ms, s.",
        "duration",
        "10ms"));
    parser.addOption(QCommandLineOption("video-bind-address", "Local RTP bind address.", "address", "0.0.0.0"));
    parser.addOption(QCommandLineOption("video-port", "Local RTP bind port.", "port", "5004"));
    parser.addOption(QCommandLineOption("telemetry-server", "Telemetry UDP server endpoint host:port.", "endpoint", "127.0.0.1:6100"));
    parser.process(application);

    const auto logLevel = ParseLogLevel(parser.value("log-level"));
    if (!logLevel.has_value())
    {
        qCritical("Invalid --log-level value. Use one of: trace, debug, info, warn, error, critical, off.");
        return 1;
    }

    const auto tickPeriod = ParseDuration(parser.value("tickrate"));
    if (!tickPeriod.has_value())
    {
        qCritical("Invalid --tickrate value. Use a positive duration with units ns, us, ms, or s.");
        return 1;
    }

    std::optional<PiSubmarine::Udp::Api::Endpoint> telemetryServerEndpoint;
    if (!parser.isSet("fake-telemetry"))
    {
        telemetryServerEndpoint = ParseEndpoint(parser.value("telemetry-server"));
        if (!telemetryServerEndpoint.has_value())
        {
            qCritical("Invalid --telemetry-server value. Use host:port.");
            return 1;
        }
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
    PiSubmarine::Operator::Station::Telemetry::View::Ballast::ViewModel ballastTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Lamp::ViewModel lampTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Depth::ViewModel depthTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Proximity::ViewModel proximityTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Video::ViewModel videoTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Battery::ViewModel batteryTelemetryViewModel;

    std::vector<std::unique_ptr<PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel>> motorTelemetryViewModels;
    std::vector<std::unique_ptr<PiSubmarine::Operator::Station::Telemetry::MotorController>> motorTelemetryControllers;
    std::vector<std::reference_wrapper<PiSubmarine::Operator::Station::Telemetry::MotorController>> motorTelemetryControllerRefs;
    QVariantList motorTelemetryViewModelList;

    motorTelemetryViewModels.reserve(DefaultTelemetryMotorCount);
    motorTelemetryControllers.reserve(DefaultTelemetryMotorCount);
    motorTelemetryControllerRefs.reserve(DefaultTelemetryMotorCount);
    motorTelemetryViewModelList.reserve(static_cast<qsizetype>(DefaultTelemetryMotorCount));

    for (std::size_t index = 0; index < DefaultTelemetryMotorCount; ++index)
    {
        motorTelemetryViewModels.push_back(
            std::make_unique<PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel>());
        motorTelemetryViewModelList.push_back(QVariant::fromValue(static_cast<QObject*>(motorTelemetryViewModels.back().get())));
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("videoViewModel", &videoViewModel);
    engine.rootContext()->setContextProperty("ballastTelemetryViewModel", &ballastTelemetryViewModel);
    engine.rootContext()->setContextProperty("inputViewModel", &inputViewModel);
    engine.rootContext()->setContextProperty("depthTelemetryViewModel", &depthTelemetryViewModel);
    engine.rootContext()->setContextProperty("lampTelemetryViewModel", &lampTelemetryViewModel);
    engine.rootContext()->setContextProperty("motorTelemetryViewModels", motorTelemetryViewModelList);
    engine.rootContext()->setContextProperty("proximityTelemetryViewModel", &proximityTelemetryViewModel);
    engine.rootContext()->setContextProperty("videoTelemetryViewModel", &videoTelemetryViewModel);
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
        // TODO replace all logger->* with SPDLOG logging macro.
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

    std::unique_ptr<PiSubmarine::Operator::Station::Composition::ITelemetry> telemetry;
    try
    {
        if (parser.isSet("fake-telemetry"))
        {
            telemetry = std::make_unique<PiSubmarine::Operator::Station::Composition::FakeTelemetry>(
                DefaultTelemetryMotorCount);
        }
        else
        {
            telemetry = std::make_unique<PiSubmarine::Operator::Station::Composition::RemoteTelemetry>(
                leaseProxy,
                *telemetryServerEndpoint,
                TelemetryReceiveQueueCapacity);
        }
    }
    catch (const std::exception& exception)
    {
        logger->error("Failed to initialize telemetry composition: {}", exception.what());
        return 1;
    }

    const auto motorTelemetryProviders = telemetry->GetMotors();

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
    auto ballastTelemetryController =
        std::make_unique<PiSubmarine::Operator::Station::Telemetry::BallastController>(telemetry->GetBallast());
    auto batteryTelemetryController =
        std::make_unique<PiSubmarine::Operator::Station::Telemetry::BatteryController>(telemetry->GetBattery());
    auto depthTelemetryController =
        std::make_unique<PiSubmarine::Operator::Station::Telemetry::DepthController>(telemetry->GetDepth());
    auto lampTelemetryController =
        std::make_unique<PiSubmarine::Operator::Station::Telemetry::LampController>(telemetry->GetLamp());
    auto proximityTelemetryController =
        std::make_unique<PiSubmarine::Operator::Station::Telemetry::ProximityController>(telemetry->GetProximity());
    auto videoStatusTelemetryController =
        std::make_unique<PiSubmarine::Operator::Station::Telemetry::VideoStatusController>(telemetry->GetVideo());

    for (auto& motorTelemetryProvider : motorTelemetryProviders)
    {
        motorTelemetryControllers.push_back(
            std::make_unique<PiSubmarine::Operator::Station::Telemetry::MotorController>(motorTelemetryProvider.get()));
        motorTelemetryControllerRefs.emplace_back(*motorTelemetryControllers.back());
    }

    auto telemetryController = std::make_unique<PiSubmarine::Operator::Station::Telemetry::Controller>(
        *lampTelemetryController,
        std::move(motorTelemetryControllerRefs),
        *batteryTelemetryController,
        *ballastTelemetryController,
        *depthTelemetryController,
        *proximityTelemetryController,
        *videoStatusTelemetryController,
        loggerFactory);
    auto inputController = std::make_unique<PiSubmarine::Operator::Station::Input::Controller>(
        fakeInputSink,
        loggerFactory);
    auto controllerTickRunner = std::make_unique<PiSubmarine::Operator::Station::Time::TickRunner>(
        *tickPeriod,
        loggerFactory);

    if (!controllerTickRunner->AddTickable(*videoController).has_value())
    {
        logger->error("Failed to add video controller to controllers tick runner");
        return 1;
    }

    for (auto& telemetryTickable : telemetry->GetTickables())
    {
        if (!controllerTickRunner->AddTickable(telemetryTickable.get()).has_value())
        {
            logger->error("Failed to add telemetry tickable to controllers tick runner");
            return 1;
        }
    }
    if (!controllerTickRunner->AddTickable(*telemetryController).has_value())
    {
        logger->error("Failed to add telemetry controller to controllers tick runner");
        return 1;
    }

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
        ballastTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::BallastController::SnapshotChanged,
        &ballastTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Ballast::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        depthTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::DepthController::SnapshotChanged,
        &depthTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Depth::ViewModel::SetSnapshot,
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
        proximityTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::ProximityController::SnapshotChanged,
        &proximityTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Proximity::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        videoStatusTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::VideoStatusController::SnapshotChanged,
        &videoTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Video::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        &inputViewModel,
        &PiSubmarine::Operator::Station::Input::View::ViewModel::IntentUpdated,
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::SubmitIntent,
        Qt::QueuedConnection);

    videoController->moveToThread(&controllerThread);
    ballastTelemetryController->moveToThread(&controllerThread);
    depthTelemetryController->moveToThread(&controllerThread);
    lampTelemetryController->moveToThread(&controllerThread);
    for (const auto& motorTelemetryController : motorTelemetryControllers)
    {
        motorTelemetryController->moveToThread(&controllerThread);
    }
    proximityTelemetryController->moveToThread(&controllerThread);
    videoStatusTelemetryController->moveToThread(&controllerThread);
    batteryTelemetryController->moveToThread(&controllerThread);
    telemetryController->moveToThread(&controllerThread);
    inputController->moveToThread(&controllerThread);
    controllerTickRunner->moveToThread(&controllerThread);

    QObject::connect(
        &controllerThread,
        &QThread::started,
        videoController.get(),
        &PiSubmarine::Operator::Station::Video::Controller::Start,
        Qt::QueuedConnection);
    QObject::connect(
        &controllerThread,
        &QThread::started,
        telemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::Controller::Start,
        Qt::QueuedConnection);
    QObject::connect(
        &controllerThread,
        &QThread::started,
        controllerTickRunner.get(),
        &PiSubmarine::Operator::Station::Time::TickRunner::Start,
        Qt::QueuedConnection);
    leaseThread.start();
    controllerThread.start();

    QObject::connect(&application, &QGuiApplication::aboutToQuit, &application, [&]()
    {
        // TODO Controllers' destruction flow looks inconsistent between each other.
        QMetaObject::invokeMethod(
            controllerTickRunner.get(),
            &PiSubmarine::Operator::Station::Time::TickRunner::Stop,
            Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(videoController.get(), &PiSubmarine::Operator::Station::Video::Controller::Stop, Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(telemetryController.get(), &PiSubmarine::Operator::Station::Telemetry::Controller::Stop, Qt::BlockingQueuedConnection);

        DeleteLaterOnObjectThread(videoController);
        DeleteLaterOnObjectThread(ballastTelemetryController);
        DeleteLaterOnObjectThread(depthTelemetryController);
        DeleteLaterOnObjectThread(lampTelemetryController);
        for (auto& motorTelemetryController : motorTelemetryControllers)
        {
            DeleteLaterOnObjectThread(motorTelemetryController);
        }
        DeleteLaterOnObjectThread(proximityTelemetryController);
        DeleteLaterOnObjectThread(videoStatusTelemetryController);
        DeleteLaterOnObjectThread(batteryTelemetryController);
        DeleteLaterOnObjectThread(telemetryController);
        DeleteLaterOnObjectThread(inputController);
        DeleteLaterOnObjectThread(controllerTickRunner);

        controllerThread.quit();
        controllerThread.wait();
        leaseThread.quit();
        leaseThread.wait();
    });

    return application.exec();
}
