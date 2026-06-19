#pragma once

#include <memory>

#include <spdlog/logger.h>

namespace PiSubmarine::Operator::Station::Logging
{
    void InstallQtMessageHandler(std::shared_ptr<spdlog::logger> logger);

    void UninstallQtMessageHandler();
}
