#pragma once

#include <memory>
#include <string>
#include <vector>

#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "PiSubmarine/Logging/Api/IFactory.h"

namespace PiSubmarine::Operator::Station::Infrastructure
{
    class SpdlogFactory final : public PiSubmarine::Logging::Api::IFactory
    {
    public:
        // TODO Is there a reason for keeping it in .h file? If no, move to SpdlogFactory.cpp.
        [[nodiscard]] std::shared_ptr<spdlog::logger> CreateLogger(std::string_view name) override
        {
            const auto loggerName = std::string(name);
            if (auto existingLogger = spdlog::get(loggerName))
            {
                return existingLogger;
            }

            static auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            static auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                "PiSubmarine.Operator.Station.log",
                true);
            const std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};

            auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());

            // TODO Make level configurable through CLI arguments
            logger->set_level(spdlog::level::info);
            logger->flush_on(spdlog::level::info);
            spdlog::register_logger(logger);
            return logger;
        }
    };
}
