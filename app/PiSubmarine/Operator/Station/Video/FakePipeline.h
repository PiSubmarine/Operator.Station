#pragma once

#include <memory>

#include <spdlog/logger.h>

#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IVideoPipelineTailFactory;

    class FakePipeline final : public GstreamerPipeline
    {
    public:
        FakePipeline(
            IVideoPipelineTailFactory& tailFactory,
            std::shared_ptr<spdlog::logger> logger);
        ~FakePipeline() override;

    private:
        [[nodiscard]] Error::Api::Result<void> BuildPipeline(IVideoPipelineTailFactory& tailFactory);
    };
}
