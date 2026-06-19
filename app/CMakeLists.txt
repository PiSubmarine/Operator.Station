include("${PISUBMARINE_BUILD_SYSTEM}")

project(${PISUBMARINE_MODULE_NAME}.App LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GSTREAMER_APP QUIET IMPORTED_TARGET gstreamer-app-1.0)
endif()

if(NOT TARGET PkgConfig::GSTREAMER_APP)
    find_library(PISUBMARINE_GSTREAMER_APP_LIBRARY
            NAMES gstapp-1.0
            PATHS
                "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/lib"
                "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/lib"
            NO_DEFAULT_PATH)
endif()

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(spdlog CONFIG REQUIRED)

PiSubmarineAddDependency("https://github.com/PiSubmarine/Battery.Telemetry.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Control.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Control.Horizontal.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Control.Lamp.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Control.Vertical.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Control.Video.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Error.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Exceptions.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Gstreamer.Build.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Lamp.Telemetry.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Lease.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Logging.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Motor.Telemetry.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Time.Tickable.Api.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Units.git" "")
PiSubmarineAddDependency("https://github.com/PiSubmarine/Video.Subscription.Api.git" "")

add_library(${PISUBMARINE_MODULE_NAME}.AppCommon STATIC
        PiSubmarine/Operator/Station/Logging/QtLog.cpp
        PiSubmarine/Operator/Station/Logging/QtLog.h
        PiSubmarine/Operator/Station/Logging/SpdlogFactory.cpp
        PiSubmarine/Operator/Station/Logging/SpdlogFactory.h
        PiSubmarine/Operator/Station/Input/Controller.cpp
        PiSubmarine/Operator/Station/Input/Controller.h
        PiSubmarine/Operator/Station/Input/FakeSink.cpp
        PiSubmarine/Operator/Station/Input/FakeSink.h
        PiSubmarine/Operator/Station/Input/View/ControlPanel.qml
        PiSubmarine/Operator/Station/Input/View/ViewModel.cpp
        PiSubmarine/Operator/Station/Input/View/ViewModel.h
        PiSubmarine/Operator/Station/Lease/FakeIssuer.cpp
        PiSubmarine/Operator/Station/Lease/FakeIssuer.h
        PiSubmarine/Operator/Station/Lease/IAsyncLeaseIssuer.h
        PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.cpp
        PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxy.h
        PiSubmarine/Operator/Station/Lease/ThreadWorker.cpp
        PiSubmarine/Operator/Station/Lease/ThreadWorker.h
        PiSubmarine/Operator/Station/Shared/Endpoint.h
        PiSubmarine/Operator/Station/Telemetry/BatteryController.cpp
        PiSubmarine/Operator/Station/Telemetry/BatteryController.h
        PiSubmarine/Operator/Station/Telemetry/Controller.cpp
        PiSubmarine/Operator/Station/Telemetry/Controller.h
        PiSubmarine/Operator/Station/Telemetry/FakeProviders.cpp
        PiSubmarine/Operator/Station/Telemetry/FakeProviders.h
        PiSubmarine/Operator/Station/Telemetry/LampController.cpp
        PiSubmarine/Operator/Station/Telemetry/LampController.h
        PiSubmarine/Operator/Station/Telemetry/MotorController.cpp
        PiSubmarine/Operator/Station/Telemetry/MotorController.h
        PiSubmarine/Operator/Station/Telemetry/View/Battery/BatteryPanel.qml
        PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.cpp
        PiSubmarine/Operator/Station/Telemetry/View/Battery/ViewModel.h
        PiSubmarine/Operator/Station/Telemetry/View/Lamp/LampPanel.qml
        PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.cpp
        PiSubmarine/Operator/Station/Telemetry/View/Lamp/ViewModel.h
        PiSubmarine/Operator/Station/Telemetry/View/Motor/MotorPanel.qml
        PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.cpp
        PiSubmarine/Operator/Station/Telemetry/View/Motor/ViewModel.h
        PiSubmarine/Operator/Station/Video/Config.h
        PiSubmarine/Operator/Station/Video/Controller.cpp
        PiSubmarine/Operator/Station/Video/Controller.h
        PiSubmarine/Operator/Station/Video/FakePipeline.cpp
        PiSubmarine/Operator/Station/Video/FakePipeline.h
        PiSubmarine/Operator/Station/Video/FakePipelineBuilder.cpp
        PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h
        PiSubmarine/Operator/Station/Video/RtpPipeline.cpp
        PiSubmarine/Operator/Station/Video/RtpPipeline.h
        PiSubmarine/Operator/Station/Video/IPipeline.h
        PiSubmarine/Operator/Station/Video/IPipelineBuilder.h
        PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h
        PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.cpp
        PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h
        PiSubmarine/Operator/Station/Video/Status.h
        PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.cpp
        PiSubmarine/Operator/Station/Video/View/QmlVideoSinkTailFactory.h
        PiSubmarine/Operator/Station/Video/View/ViewModel.cpp
        PiSubmarine/Operator/Station/Video/View/ViewModel.h
        PiSubmarine/Operator/Station/Video/View/VideoSurfaceItem.cpp
        PiSubmarine/Operator/Station/Video/View/VideoSurfaceItem.h
        PiSubmarine/Operator/Station/View/qml.qrc
        PiSubmarine/Operator/Station/View/qml/Main.qml
        PiSubmarine/Operator/Station/View/qml/Main.Windows.qml
        PiSubmarine/Operator/Station/Video/GstreamerPipeline.cpp
        PiSubmarine/Operator/Station/Video/GstreamerPipeline.h)
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME}.AppCommon)
target_include_directories(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC ./)
target_compile_options(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC spdlog::spdlog)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Battery.Telemetry.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Control.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Control.Horizontal.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Control.Lamp.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Control.Vertical.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Control.Video.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Error.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Exceptions)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Gstreamer.Build)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Lamp.Telemetry.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Lease.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Logging.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Motor.Telemetry.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Time.Tickable.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Units)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.AppCommon PUBLIC PiSubmarine.Video.Subscription.Api)

foreach(_gstreamer_plugin IN ITEMS
        app
        coreelements
        autodetect
        videotestsrc
        videoconvertscale
        videoparsersbad
        openh264
        rtp
        udp)
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME}.AppCommon ${_gstreamer_plugin})
endforeach()

if(WIN32)
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME}.AppCommon d3d11)
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME}.AppCommon mediafoundation)
else()
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME}.AppCommon video4linux2)
endif()

add_executable(${PISUBMARINE_MODULE_NAME}.App
        PiSubmarine/Operator/Station/main.cpp)
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME}.App)
target_include_directories(${PISUBMARINE_MODULE_NAME}.App PRIVATE ./)
target_compile_options(${PISUBMARINE_MODULE_NAME}.App PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE ${PISUBMARINE_MODULE_NAME}.AppCommon)
if(TARGET PkgConfig::GSTREAMER_APP)
    target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE PkgConfig::GSTREAMER_APP)
elseif(PISUBMARINE_GSTREAMER_APP_LIBRARY)
    target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE ${PISUBMARINE_GSTREAMER_APP_LIBRARY})
else()
    message(FATAL_ERROR "Failed to locate gstreamer-app-1.0 library for ${PISUBMARINE_MODULE_NAME}.App")
endif()

qt_import_qml_plugins(${PISUBMARINE_MODULE_NAME}.App)
qt_finalize_executable(${PISUBMARINE_MODULE_NAME}.App)
