cmake_minimum_required(VERSION 3.25)

include(FetchContent)

FetchContent_Declare(
        PiSubmarineBuild
        GIT_REPOSITORY "https://github.com/PiSubmarine/Build.CMake.git"
        GIT_TAG        "main"
)

FetchContent_MakeAvailable(PiSubmarineBuild)

set(PISUBMARINE_BUILD_SYSTEM "${pisubmarinebuild_SOURCE_DIR}/PiSubmarine.Build.CMake.cmake")

# For legacy modules
set(_build_cmake_local "${pisubmarinebuild_SOURCE_DIR}/PiSubmarine.Build.CMake.cmake")

include("${pisubmarinebuild_SOURCE_DIR}/PiSubmarine.Build.CMake.cmake")

PiSubmarineInitModule("")
project(${PISUBMARINE_MODULE_NAME})
enable_testing()
PiSubmarineAddDependency("https://github.com/PiSubmarine/Gstreamer.Build.git" "")
PiSubmarineConfigureModule()

if(WIN32 AND TARGET ${PISUBMARINE_MODULE_NAME}.App)
    PiSubmarineGstreamerAddPlugin(${PISUBMARINE_MODULE_NAME}.App qt6d3d11)
endif()
if(TARGET ${PISUBMARINE_MODULE_NAME}.App)
    PiSubmarineGstreamerFinalizeCompositionRoot(${PISUBMARINE_MODULE_NAME}.App)
endif()
