#include "PiSubmarine/Operator/Station/Logging/SpdlogFactory.h"

#include <string>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace PiSubmarine::Operator::Station::Logging
{
    SpdlogFactory::SpdlogFactory(const spdlog::level::level_enum level)
        : m_Level(level)
    {
    }

    std::shared_ptr<spdlog::logger> SpdlogFactory::CreateLogger(std::string_view name)
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
        logger->set_level(m_Level);
        logger->flush_on(m_Level);
        spdlog::register_logger(logger);
        return logger;
    }
}
