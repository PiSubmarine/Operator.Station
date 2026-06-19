#pragma once

#include <memory>

#include "PiSubmarine/Operator/Station/Video/Config.h"
namespace PiSubmarine::Operator::Station::Video
{
    class IPipeline;

    class IPipelineBuilder
    {
    public:
        virtual ~IPipelineBuilder() = default;

        [[nodiscard]] virtual std::unique_ptr<IPipeline> Build(const ReceiveEndpoint& receiveEndpoint) = 0;
    };
}
