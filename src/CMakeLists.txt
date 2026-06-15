include("${PISUBMARINE_BUILD_SYSTEM}")

find_package(spdlog CONFIG REQUIRED)

message("Adding library: ${PISUBMARINE_MODULE_NAME}")
add_library(${PISUBMARINE_MODULE_NAME}
        PiSubmarine/Operator/Station/Video/FakePipelineBuilder.cpp
        PiSubmarine/Operator/Station/Video/GstreamerPipeline.cpp
        PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.cpp
        PiSubmarine/Operator/Station/Video/VideoController.cpp
        "../public/PiSubmarine/Operator/Station/Video/Config.h"
        "../public/PiSubmarine/Operator/Station/Video/FakePipelineBuilder.h"
        "../public/PiSubmarine/Operator/Station/Video/IPipelineBuilder.h"
        "../public/PiSubmarine/Operator/Station/Video/IVideoPipelineTailFactory.h"
        "../public/PiSubmarine/Operator/Station/Video/RtpPipelineBuilder.h"
        "../public/PiSubmarine/Operator/Station/Video/Status.h"
        "../public/PiSubmarine/Operator/Station/Video/VideoController.h"
        "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"
        "PiSubmarine/Operator/Station/Video/IPipeline.h")
target_include_directories(${PISUBMARINE_MODULE_NAME}
        PUBLIC "../public"
        PRIVATE "./")
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME})
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC spdlog::spdlog)

target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Gstreamer.Build)

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
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME} ${_gstreamer_plugin})
endforeach()

if(WIN32)
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME} mediafoundation)
else()
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME} video4linux2)
endif()

PiSubmarineAddDependency("https://github.com/PiSubmarine/Error.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Error.Api)

PiSubmarineAddDependency("https://github.com/PiSubmarine/Lease.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Lease.Api)

PiSubmarineAddDependency("https://github.com/PiSubmarine/Logging.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Logging.Api)

PiSubmarineAddDependency("https://github.com/PiSubmarine/Time.Tickable.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Time.Tickable.Api)

PiSubmarineAddDependency("https://github.com/PiSubmarine/Video.Subscription.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Video.Subscription.Api)
