#pragma once

#include <memory>
#include <string_view>

#include <spdlog/common.h>
#include <spdlog/logger.h>

#include "PiSubmarine/Logging/Api/IFactory.h"

namespace PiSubmarine::Operator::Station::Logging
{
    class SpdlogFactory final : public PiSubmarine::Logging::Api::IFactory
    {
    public:
        explicit SpdlogFactory(spdlog::level::level_enum level = spdlog::level::info);

        [[nodiscard]] std::shared_ptr<spdlog::logger> CreateLogger(std::string_view name) override;

    private:
        spdlog::level::level_enum m_Level;
    };
}
