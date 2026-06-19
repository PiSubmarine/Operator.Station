#pragma once

#include <memory>

#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Video/IPipelineBuilder.h"

namespace PiSubmarine::Operator::Station::Video
{
    [[nodiscard]] std::shared_ptr<IPipelineBuilder> CreateFakePipelineBuilder(PiSubmarine::Logging::Api::IFactory& loggerFactory);
}
