#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQuickItem>
#include <QRegularExpression>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QThread>
#include <QUrl>
#include <QVariant>

#include <gst/gst.h>
#include <PiSubmarine/Operator/Station/Telemetry/BallastController.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "PiSubmarine/Error/Api/Result.h"
#include "PiSubmarine/Operator/Station/Composition/FakeControl.h"
#include "PiSubmarine/Operator/Station/Composition/FakeInput.h"
#include "PiSubmarine/Operator/Station/Composition/FakeLease.h"
#include "PiSubmarine/Operator/Station/Composition/FakeTelemetry.h"
#include "PiSubmarine/Operator/Station/Composition/FakeVideo.h"
#include "PiSubmarine/Operator/Station/Composition/IControl.h"
#include "PiSubmarine/Operator/Station/Composition/IInput.h"
#include "PiSubmarine/Operator/Station/Composition/ILease.h"
#include "PiSubmarine/Operator/Station/Composition/ITelemetry.h"
#include "PiSubmarine/Operator/Station/Composition/IVideo.h"
#include "PiSubmarine/Operator/Station/Composition/RemoteControl.h"
#include "PiSubmarine/Operator/Station/Composition/RemoteLease.h"
#include "PiSubmarine/Operator/Station/Composition/RemoteTelemetry.h"
#include "PiSubmarine/Operator/Station/Composition/RemoteVideo.h"
#include "PiSubmarine/Operator/Station/Control/Controller.h"
#include "PiSubmarine/Operator/Station/Control/View/StatusViewModel.h"
#include "PiSubmarine/Operator/Station/Control/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Input/BindingDescriptor.h"
#include "PiSubmarine/Operator/Station/Input/Controller.h"
#include "PiSubmarine/Operator/Station/Input/View/BindingViewModel.h"
#include "PiSubmarine/Operator/Station/Logging/QtLog.h"
#include "PiSubmarine/Operator/Station/Logging/SpdlogFactory.h"
#include "PiSubmarine/Operator/Station/Telemetry/BallastController.h"
#include "PiSubmarine/Operator/Station/Telemetry/BatteryController.h"
#include "PiSubmarine/Operator/Station/Telemetry/LampController.h"
#include "PiSubmarine/Operator/Station/Telemetry/DepthController.h"
#include "PiSubmarine/Operator/Station/Telemetry/MotorController.h"
#include "PiSubmarine/Operator/Station/Telemetry/ProximityController.h"
#include "PiSubmarine/Operator/Station/Telemetry/TimeController.h"
#include "PiSubmarine/Operator/Station/Telemetry/Controller.h"
#include "PiSubmarine/Operator/Station/Telemetry/VideoStatusController.h"
#include "PiSubmarine/Operator/Station/Time/TickRunner.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Ballast/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Depth/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Proximity/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Time/ViewModel.h"
#include "PiSubmarine/Operator/Station/Telemetry/View/Video/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/Controller.h"
#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"
#include "PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h"
#include "PiSubmarine/Operator/Station/Video/View/StatusOverlayViewModel.h"
#include "PiSubmarine/Operator/Station/Video/View/ViewModel.h"
#include "PiSubmarine/Operator/Station/Video/View/VideoSurfaceItem.h"
#include "PiSubmarine/Udp/Api/Endpoint.h"

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

    constexpr std::size_t ControlReceiveQueueCapacity = 1;
    constexpr std::size_t TelemetryReceiveQueueCapacity = 16;
    constexpr std::size_t DefaultTelemetryMotorCount = 4;
    const std::vector<PiSubmarine::Operator::Station::Input::BindingDescriptor> DefaultInputBindings{
        {.Name = "Surge", .Type = PiSubmarine::Operator::Station::Input::BindingType::Axis},
        {.Name = "Yaw", .Type = PiSubmarine::Operator::Station::Input::BindingType::Axis},
        {.Name = "Ballast", .Type = PiSubmarine::Operator::Station::Input::BindingType::Axis},
        {.Name = "Lamp", .Type = PiSubmarine::Operator::Station::Input::BindingType::Axis},
        {.Name = "Hold Position", .Type = PiSubmarine::Operator::Station::Input::BindingType::Key}
    };

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

    [[nodiscard]] QQuickItem* AttachVideoSurfaceItem(
        QQmlApplicationEngine& engine,
        QQuickItem& videoSurfaceHost,
        const std::shared_ptr<spdlog::logger>& logger)
    {
        auto resizeToHost = [&videoSurfaceHost](QQuickItem& item)
        {
            item.setParent(&videoSurfaceHost);
            item.setParentItem(&videoSurfaceHost);
            item.setX(0);
            item.setY(0);
            item.setWidth(videoSurfaceHost.width());
            item.setHeight(videoSurfaceHost.height());

            QObject::connect(&videoSurfaceHost, &QQuickItem::widthChanged, &item, [&videoSurfaceHost, &item]()
            {
                item.setWidth(videoSurfaceHost.width());
            });
            QObject::connect(&videoSurfaceHost, &QQuickItem::heightChanged, &item, [&videoSurfaceHost, &item]()
            {
                item.setHeight(videoSurfaceHost.height());
            });
        };

#if defined(_WIN32)
        QQmlComponent component(
            &engine,
            QUrl("qrc:/PiSubmarine/Operator/Station/View/WindowsVideoSurface.qml"));

        if (component.isError())
        {
            for (const auto& error : component.errors())
            {
                SPDLOG_LOGGER_ERROR(logger, "Failed to prepare Windows video surface component: {}", error.toString().toStdString());
            }
            return nullptr;
        }

        auto* object = component.create(engine.rootContext());
        auto* item = qobject_cast<QQuickItem*>(object);
        if (item == nullptr)
        {
            SPDLOG_LOGGER_ERROR(logger, "Windows video surface component did not create a QQuickItem");
            delete object;
            return nullptr;
        }

        resizeToHost(*item);
        return item;
#else
        auto* item = new PiSubmarine::Operator::Station::Video::View::VideoSurfaceItem(&videoSurfaceHost);
        item->setObjectName("videoSurface");
        resizeToHost(*item);
        return item;
#endif
    }
}

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(qml);

    QGuiApplication application(argc, argv);
    qRegisterMetaType<PiSubmarine::Operator::Station::Video::Status>("PiSubmarine::Operator::Station::Video::Status");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("fake-control", "Use a local fake control sink."));
    parser.addOption(QCommandLineOption("fake-lease", "Use a local fake lease issuer."));
    parser.addOption(QCommandLineOption("fake-telemetry", "Use local fake telemetry providers."));
    parser.addOption(QCommandLineOption("fake-video", "Use a local fake test-pattern video pipeline."));
    parser.addOption(QCommandLineOption(
        "grpc-server",
        "Shared gRPC server endpoint host:port.",
        "endpoint",
        "127.0.0.1:50051"));
    parser.addOption(QCommandLineOption(
        "log-level",
        "Set application log level: trace, debug, info, warn, error, critical, off.",
        "level",
        "info"));
    parser.addOption(QCommandLineOption("tls-ca", "PEM client certificate authority file.", "path"));
    parser.addOption(QCommandLineOption("tls-cert", "PEM client certificate chain file.", "path"));
    parser.addOption(QCommandLineOption("tls-key", "PEM client private key file.", "path"));
    parser.addOption(QCommandLineOption("tls-server-authority", "Optional TLS server authority override.", "name", ""));
    parser.addOption(QCommandLineOption(
        "control-server",
        "Control UDP server endpoint host:port.",
        "endpoint",
        "127.0.0.1:50052"));
    parser.addOption(QCommandLineOption(
        "tickrate",
        "Set controllers thread tick period. Supported units: ns, us, ms, s.",
        "duration",
        "10ms"));
    parser.addOption(QCommandLineOption("video-bind", "Local RTP bind endpoint host:port.", "endpoint", "0.0.0.0:5005"));
    parser.addOption(QCommandLineOption("telemetry-server", "Telemetry UDP server endpoint host:port.", "endpoint", "127.0.0.1:50053"));
    parser.process(application);

    const auto logLevel = ParseLogLevel(parser.value("log-level"));
    if (!logLevel.has_value())
    {
        std::fprintf(
            stderr,
            "Invalid --log-level value. Use one of: trace, debug, info, warn, error, critical, off.\n");
        return 1;
    }

    PiSubmarine::Operator::Station::Logging::SpdlogFactory loggerFactory(*logLevel);
    auto qtLogger = loggerFactory.CreateLogger("Qt");
    PiSubmarine::Operator::Station::Logging::InstallQtMessageHandler(qtLogger);
    const auto logger = loggerFactory.CreateLogger("Operator.Station.Main");

    const auto tickPeriod = ParseDuration(parser.value("tickrate"));
    if (!tickPeriod.has_value())
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Invalid --tickrate value. Use a positive duration with units ns, us, ms, or s.");
        return 1;
    }

    QString grpcServer;
    std::filesystem::path tlsCaPath;
    std::filesystem::path tlsCertPath;
    std::filesystem::path tlsKeyPath;
    QString tlsServerAuthorityOverride;
    if (!parser.isSet("fake-lease") || !parser.isSet("fake-video"))
    {
        grpcServer = parser.value("grpc-server");
        if (ParseEndpoint(grpcServer).has_value() == false)
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Invalid --grpc-server value. Use host:port.");
            return 1;
        }

        tlsCaPath = parser.value("tls-ca").toStdString();
        tlsCertPath = parser.value("tls-cert").toStdString();
        tlsKeyPath = parser.value("tls-key").toStdString();
        tlsServerAuthorityOverride = parser.value("tls-server-authority");
        if (tlsCaPath.empty() || tlsCertPath.empty() || tlsKeyPath.empty())
        {
            SPDLOG_LOGGER_CRITICAL(
                logger,
                "Remote gRPC clients require --tls-ca, --tls-cert, and --tls-key.");
            return 1;
        }
    }

    std::optional<PiSubmarine::Udp::Api::Endpoint> controlServerEndpoint;
    if (!parser.isSet("fake-control"))
    {
        controlServerEndpoint = ParseEndpoint(parser.value("control-server"));
        if (!controlServerEndpoint.has_value())
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Invalid --control-server value. Use host:port.");
            return 1;
        }
    }

    std::optional<PiSubmarine::Udp::Api::Endpoint> telemetryServerEndpoint;
    if (!parser.isSet("fake-telemetry"))
    {
        telemetryServerEndpoint = ParseEndpoint(parser.value("telemetry-server"));
        if (!telemetryServerEndpoint.has_value())
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Invalid --telemetry-server value. Use host:port.");
            return 1;
        }
    }

    const auto videoBindEndpoint = ParseEndpoint(parser.value("video-bind"));
    if (!videoBindEndpoint.has_value())
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Invalid --video-bind value. Use host:port.");
        return 1;
    }

    if (!logger || !PiSubmarine::Operator::Station::Video::GstreamerPipeline::EnsureGstreamerInitialized(logger))
    {
        return 1;
    }

    PiSubmarine::Operator::Station::Video::View::ViewModel videoViewModel;
    PiSubmarine::Operator::Station::Video::View::StatusOverlayViewModel videoStatusOverlayViewModel;
    videoViewModel.SetReceiveBindAddress(QString::fromStdString(videoBindEndpoint->Address));
    videoViewModel.SetReceivePort(videoBindEndpoint->Port);
    videoViewModel.SetSubscriptionHost("127.0.0.1");
    videoViewModel.SetSubscriptionPort(videoBindEndpoint->Port);

    PiSubmarine::Operator::Station::Control::View::ViewModel controlViewModel;
    PiSubmarine::Operator::Station::Control::View::StatusViewModel controlStatusViewModel;
    PiSubmarine::Operator::Station::Input::View::BindingViewModel inputBindingViewModel(DefaultInputBindings);
    PiSubmarine::Operator::Station::Telemetry::View::Ballast::ViewModel ballastTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Lamp::ViewModel lampTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Depth::ViewModel depthTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Proximity::ViewModel proximityTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Time::ViewModel timeTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Video::ViewModel videoTelemetryViewModel;
    PiSubmarine::Operator::Station::Telemetry::View::Battery::ViewModel batteryTelemetryViewModel;

    std::vector<std::unique_ptr<PiSubmarine::Operator::Station::Telemetry::View::Motor::ViewModel>> motorTelemetryViewModels;
    std::vector<std::unique_ptr<PiSubmarine::Operator::Station::Telemetry::MotorController>> motorTelemetryControllers;
    std::vector<std::reference_wrapper<PiSubmarine::Operator::Station::Telemetry::MotorController>> motorTelemetryControllerRefs;
    QVariantList motorTelemetryViewModelList;
    QVariantList videoOverlayViewModelList;

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

    // TODO Can we shift responsibility of adding overlays to ViewModels? This is a question, I don't know if this is a good idea.
    videoOverlayViewModelList.push_back(QVariant::fromValue(static_cast<QObject*>(&videoStatusOverlayViewModel)));
    videoOverlayViewModelList.push_back(QVariant::fromValue(static_cast<QObject*>(&videoTelemetryViewModel)));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("videoViewModel", &videoViewModel);
    engine.rootContext()->setContextProperty("ballastTelemetryViewModel", &ballastTelemetryViewModel);
    engine.rootContext()->setContextProperty("controlViewModel", &controlViewModel);
    engine.rootContext()->setContextProperty("controlStatusViewModel", &controlStatusViewModel);
    engine.rootContext()->setContextProperty("depthTelemetryViewModel", &depthTelemetryViewModel);
    engine.rootContext()->setContextProperty("inputBindingViewModel", &inputBindingViewModel);
    engine.rootContext()->setContextProperty("lampTelemetryViewModel", &lampTelemetryViewModel);
    engine.rootContext()->setContextProperty("motorTelemetryViewModels", motorTelemetryViewModelList);
    engine.rootContext()->setContextProperty("proximityTelemetryViewModel", &proximityTelemetryViewModel);
    engine.rootContext()->setContextProperty("timeTelemetryViewModel", &timeTelemetryViewModel);
    engine.rootContext()->setContextProperty("videoTelemetryViewModel", &videoTelemetryViewModel);
    engine.rootContext()->setContextProperty("videoOverlayViewModels", videoOverlayViewModelList);
    engine.rootContext()->setContextProperty("batteryTelemetryViewModel", &batteryTelemetryViewModel);
    const QUrl mainWindowUrl("qrc:/PiSubmarine/Operator/Station/qml/Main.qml");
    engine.load(mainWindowUrl);
    if (engine.rootObjects().isEmpty())
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to load main QML window");
        return 1;
    }

    auto* videoSurfaceHost = engine.rootObjects().front()->findChild<QQuickItem*>("videoSurfaceHost");
    if (videoSurfaceHost == nullptr)
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to locate QML video surface host item");
        return 1;
    }

    auto* videoItem = AttachVideoSurfaceItem(engine, *videoSurfaceHost, logger);
    if (videoItem == nullptr)
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to create platform video surface item");
        return 1;
    }

    QThread controllerThread;
    QThread leaseThread;

    std::unique_ptr<PiSubmarine::Operator::Station::Composition::ILease> lease;
    try
    {
        if (parser.isSet("fake-lease"))
        {
            lease = std::make_unique<PiSubmarine::Operator::Station::Composition::FakeLease>(loggerFactory);
        }
        else
        {
            lease = std::make_unique<PiSubmarine::Operator::Station::Composition::RemoteLease>(
                loggerFactory,
                PiSubmarine::Operator::Station::Composition::RemoteLeaseConfig{
                    .GrpcTarget = grpcServer.toStdString(),
                    .CertificateAuthorityPath = tlsCaPath,
                    .ClientCertificateChainPath = tlsCertPath,
                    .ClientPrivateKeyPath = tlsKeyPath,
                    .ServerAuthorityOverride = tlsServerAuthorityOverride.toStdString()});
        }
    }
    catch (const std::exception& exception)
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to initialize lease composition: {}", exception.what());
        return 1;
    }
    lease->GetWorkerObject().moveToThread(&leaseThread);

    PiSubmarine::Operator::Station::Video::View::QmlVideoSinkTailFactory videoTailFactory(*videoItem, logger);

    std::unique_ptr<PiSubmarine::Operator::Station::Composition::IVideo> video;
    try
    {
        if (parser.isSet("fake-video"))
        {
            video = std::make_unique<PiSubmarine::Operator::Station::Composition::FakeVideo>(
                loggerFactory,
                videoTailFactory);
        }
        else
        {
            video = std::make_unique<PiSubmarine::Operator::Station::Composition::RemoteVideo>(
                loggerFactory,
                videoTailFactory,
                PiSubmarine::Operator::Station::Composition::RemoteVideoConfig{
                    .GrpcTarget = grpcServer.toStdString(),
                    .CertificateAuthorityPath = tlsCaPath,
                    .ClientCertificateChainPath = tlsCertPath,
                    .ClientPrivateKeyPath = tlsKeyPath,
                    .ServerAuthorityOverride = tlsServerAuthorityOverride.toStdString()});
        }
    }
    catch (const std::exception& exception)
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to initialize video composition: {}", exception.what());
        return 1;
    }

    std::unique_ptr<PiSubmarine::Operator::Station::Composition::IControl> control;
    try
    {
        if (parser.isSet("fake-control"))
        {
            control = std::make_unique<PiSubmarine::Operator::Station::Composition::FakeControl>();
        }
        else
        {
            control = std::make_unique<PiSubmarine::Operator::Station::Composition::RemoteControl>(
                lease->GetIssuer(),
                *controlServerEndpoint,
                ControlReceiveQueueCapacity);
        }
    }
    catch (const std::exception& exception)
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to initialize control composition: {}", exception.what());
        return 1;
    }

    auto input = std::make_unique<PiSubmarine::Operator::Station::Composition::FakeInput>();
    const auto inputBindingFilePath = std::filesystem::current_path() / "PiSubmarine.Operator.Station.InputBindings.txt";

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
                lease->GetIssuer(),
                *telemetryServerEndpoint,
                TelemetryReceiveQueueCapacity);
        }
    }
    catch (const std::exception& exception)
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to initialize telemetry composition: {}", exception.what());
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

    auto videoController = std::make_unique<PiSubmarine::Operator::Station::Video::Controller>(
        videoConfig,
        loggerFactory,
        lease->GetIssuer(),
        video->GetSubscriptionService(),
        video->GetPipelineBuilder());
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
    auto timeTelemetryController =
        std::make_unique<PiSubmarine::Operator::Station::Telemetry::TimeController>(
            telemetry->GetTime(),
            [telemetry = telemetry.get()] { return telemetry->HasLease(); });
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
        *timeTelemetryController,
        *videoStatusTelemetryController,
        loggerFactory);
    auto controlController = std::make_unique<PiSubmarine::Operator::Station::Control::Controller>(
        control->GetSink(),
        loggerFactory);
    auto inputController = std::make_unique<PiSubmarine::Operator::Station::Input::Controller>(
        input->GetManager(),
        input->GetBinder(),
        input->GetSerializer(),
        inputBindingFilePath,
        DefaultInputBindings);
    auto controllerTickRunner = std::make_unique<PiSubmarine::Operator::Station::Time::TickRunner>(
        *tickPeriod,
        loggerFactory);

    if (!controllerTickRunner->AddTickable(*videoController).has_value())
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to add video controller to controllers tick runner");
        return 1;
    }

    for (auto& controlTickable : control->GetTickables())
    {
        if (!controllerTickRunner->AddTickable(controlTickable.get()).has_value())
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Failed to add control tickable to controllers tick runner");
            return 1;
        }
    }

    for (auto& telemetryTickable : telemetry->GetTickables())
    {
        if (!controllerTickRunner->AddTickable(telemetryTickable.get()).has_value())
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Failed to add telemetry tickable to controllers tick runner");
            return 1;
        }
    }
    for (auto& inputTickable : input->GetTickables())
    {
        if (!controllerTickRunner->AddTickable(inputTickable.get()).has_value())
        {
            SPDLOG_LOGGER_CRITICAL(logger, "Failed to add input tickable to controllers tick runner");
            return 1;
        }
    }
    if (!controllerTickRunner->AddTickable(*telemetryController).has_value())
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to add telemetry controller to controllers tick runner");
        return 1;
    }
    if (!controllerTickRunner->AddTickable(*controlController).has_value())
    {
        SPDLOG_LOGGER_CRITICAL(logger, "Failed to add control controller to controllers tick runner");
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
        videoController.get(),
        &PiSubmarine::Operator::Station::Video::Controller::StatusChanged,
        &videoStatusOverlayViewModel,
        &PiSubmarine::Operator::Station::Video::View::StatusOverlayViewModel::SetStatus,
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
        timeTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::TimeController::SnapshotChanged,
        &timeTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Time::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        timeTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::TimeController::SnapshotChanged,
        &controlStatusViewModel,
        [viewModel = &controlStatusViewModel](const bool hasLease, const QString&, const QString&)
        {
            viewModel->SetLeaseState(hasLease);
        },
        Qt::QueuedConnection);
    QObject::connect(
        videoStatusTelemetryController.get(),
        &PiSubmarine::Operator::Station::Telemetry::VideoStatusController::SnapshotChanged,
        &videoTelemetryViewModel,
        &PiSubmarine::Operator::Station::Telemetry::View::Video::ViewModel::SetSnapshot,
        Qt::QueuedConnection);
    QObject::connect(
        &controlViewModel,
        &PiSubmarine::Operator::Station::Control::View::ViewModel::IntentUpdated,
        controlController.get(),
        &PiSubmarine::Operator::Station::Control::Controller::SubmitIntent,
        Qt::QueuedConnection);
    QObject::connect(
        &inputBindingViewModel,
        &PiSubmarine::Operator::Station::Input::View::BindingViewModel::RequestCapture,
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::Capture,
        Qt::QueuedConnection);
    QObject::connect(
        &inputBindingViewModel,
        &PiSubmarine::Operator::Station::Input::View::BindingViewModel::RequestCancelCapture,
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::CancelCapture,
        Qt::QueuedConnection);
    QObject::connect(
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::BindingHintChanged,
        &inputBindingViewModel,
        &PiSubmarine::Operator::Station::Input::View::BindingViewModel::SetBindingHint,
        Qt::QueuedConnection);
    QObject::connect(
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::AllBindingsConfiguredChanged,
        &controlStatusViewModel,
        &PiSubmarine::Operator::Station::Control::View::StatusViewModel::SetAllBindingsConfigured,
        Qt::QueuedConnection);
    QObject::connect(
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::CaptureInProgressChanged,
        &inputBindingViewModel,
        &PiSubmarine::Operator::Station::Input::View::BindingViewModel::SetCaptureInProgress,
        Qt::QueuedConnection);
    QObject::connect(
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::StatusMessageChanged,
        &inputBindingViewModel,
        &PiSubmarine::Operator::Station::Input::View::BindingViewModel::SetStatusMessage,
        Qt::QueuedConnection);
    QObject::connect(
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::OnAxisBound,
        controlController.get(),
        [control = controlController.get()](const QString& name, ::PiSubmarine::Input::Api::IAxis* axis)
        {
            if (name == "Surge")
            {
                control->SetSurgeAxis(axis);
                return;
            }
            if (name == "Yaw")
            {
                control->SetYawAxis(axis);
                return;
            }
            if (name == "Ballast")
            {
                control->SetBallastAxis(axis);
                return;
            }
            if (name == "Lamp")
            {
                control->SetLampAxis(axis);
            }
        });
    QObject::connect(
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::OnKeyBound,
        controlController.get(),
        [control = controlController.get()](const QString& name, ::PiSubmarine::Input::Api::IKey* key)
        {
            if (name == "Hold Position")
            {
                control->SetHoldPositionKey(key);
            }
        });

    videoController->moveToThread(&controllerThread);
    ballastTelemetryController->moveToThread(&controllerThread);
    depthTelemetryController->moveToThread(&controllerThread);
    lampTelemetryController->moveToThread(&controllerThread);
    for (const auto& motorTelemetryController : motorTelemetryControllers)
    {
        motorTelemetryController->moveToThread(&controllerThread);
    }
    proximityTelemetryController->moveToThread(&controllerThread);
    timeTelemetryController->moveToThread(&controllerThread);
    videoStatusTelemetryController->moveToThread(&controllerThread);
    batteryTelemetryController->moveToThread(&controllerThread);
    telemetryController->moveToThread(&controllerThread);
    controlController->moveToThread(&controllerThread);
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
        inputController.get(),
        &PiSubmarine::Operator::Station::Input::Controller::Start,
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
        DeleteLaterOnObjectThread(timeTelemetryController);
        DeleteLaterOnObjectThread(videoStatusTelemetryController);
        DeleteLaterOnObjectThread(batteryTelemetryController);
        DeleteLaterOnObjectThread(telemetryController);
        DeleteLaterOnObjectThread(controlController);
        DeleteLaterOnObjectThread(inputController);
        DeleteLaterOnObjectThread(controllerTickRunner);

        controllerThread.quit();
        controllerThread.wait();

        video.reset();
        control.reset();
        input.reset();

        // Telemetry owns the UDP client that releases its lease in the destructor.
        // Destroy it before the lease worker shuts down so the release request can still be processed.
        telemetry.reset();

        leaseThread.quit();
        leaseThread.wait();
        lease.reset();
    });

    return QGuiApplication::exec();
}
