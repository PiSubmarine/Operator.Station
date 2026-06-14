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

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(spdlog CONFIG REQUIRED)

message("Adding executable: ${PISUBMARINE_MODULE_NAME}.App")
add_executable(${PISUBMARINE_MODULE_NAME}.App
        PiSubmarine/Operator/Station/main.cpp
        PiSubmarine/Operator/Station/Qt/StationApp.cpp
        PiSubmarine/Operator/Station/Qt/StationApp.h
        PiSubmarine/Operator/Station/Qt/VideoItem.cpp
        PiSubmarine/Operator/Station/Qt/VideoItem.h
        PiSubmarine/Operator/Station/Qt/VideoRuntimeWorker.cpp
        PiSubmarine/Operator/Station/Qt/VideoRuntimeWorker.h
        PiSubmarine/Operator/Station/Qt/VideoTailFactory.cpp
        PiSubmarine/Operator/Station/Qt/VideoTailFactory.h
        PiSubmarine/Operator/Station/Qt/qml.qrc)
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME}.App)

target_include_directories(${PISUBMARINE_MODULE_NAME}.App PRIVATE ./)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE ${PISUBMARINE_MODULE_NAME})
target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE spdlog::spdlog)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)
if(TARGET PkgConfig::GSTREAMER_APP)
    target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE PkgConfig::GSTREAMER_APP)
elseif(PISUBMARINE_GSTREAMER_APP_LIBRARY)
    target_link_libraries(${PISUBMARINE_MODULE_NAME}.App PRIVATE ${PISUBMARINE_GSTREAMER_APP_LIBRARY})
else()
    message(FATAL_ERROR "Failed to locate gstreamer-app-1.0 library for ${PISUBMARINE_MODULE_NAME}.App")
endif()

qt_import_qml_plugins(${PISUBMARINE_MODULE_NAME}.App)
qt_finalize_executable(${PISUBMARINE_MODULE_NAME}.App)
