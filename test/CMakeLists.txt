cmake_minimum_required(VERSION 3.20)

include(FetchContent)

project(${PISUBMARINE_MODULE_NAME}.Test LANGUAGES CXX)

set(${PISUBMARINE_MODULE_NAME}.Test.Sources
        "PiSubmarine/Operator/Station/Lease/SyncLeaseIssuerProxyTest.cpp"
        "PiSubmarine/Operator/Station/Input/AxisMultiplexorTest.cpp"
        "PiSubmarine/Operator/Station/Input/KeyMultiplexorTest.cpp"
        "PiSubmarine/Operator/Station/Control/ControllerTest.cpp"
        "PiSubmarine/Operator/Station/Video/ControllerTest.cpp")

enable_testing()

find_package(spdlog)
find_package(GTest)

add_executable(${PISUBMARINE_MODULE_NAME}.Test ${${PISUBMARINE_MODULE_NAME}.Test.Sources})
target_include_directories(${PISUBMARINE_MODULE_NAME}.Test PRIVATE "../app")
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME}.Test)

target_link_libraries(${PISUBMARINE_MODULE_NAME}.Test PRIVATE ${PISUBMARINE_MODULE_NAME}.AppCommon)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.Test PRIVATE PiSubmarine.Control.Api.Mock)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.Test PRIVATE PiSubmarine.Lease.Api.Mock)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.Test PRIVATE PiSubmarine.Video.Subscription.Api.Mock)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.Test PRIVATE spdlog::spdlog)
target_link_libraries(${PISUBMARINE_MODULE_NAME}.Test PRIVATE GTest::gtest_main)
include(GoogleTest)
if(CMAKE_CROSSCOMPILING)
    gtest_add_tests(TARGET ${PISUBMARINE_MODULE_NAME}.Test)
else()
    gtest_discover_tests(${PISUBMARINE_MODULE_NAME}.Test)
endif()
