#pragma once

#include <memory>

#include <spdlog/logger.h>

namespace PiSubmarine::Operator::Station::Video
{
    void RegisterStaticPlugins(const std::shared_ptr<spdlog::logger>& logger);
}
