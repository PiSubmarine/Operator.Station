#pragma once

#include <memory>

#include "PiSubmarine/Logging/Api/IFactory.h"
#include "PiSubmarine/Operator/Station/Video/Config.h"
#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IVideoPipelineTailFactory;

    class RtpPipeline final : public GstreamerPipeline
    {
    public:
        RtpPipeline(
            const ReceiveEndpoint& receiveEndpoint,
            IVideoPipelineTailFactory& tailFactory,
            PiSubmarine::Logging::Api::IFactory& loggerFactory);
        ~RtpPipeline() override;

    private:
        [[nodiscard]] Error::Api::Result<void> BuildPipeline(
            const ReceiveEndpoint& receiveEndpoint,
            IVideoPipelineTailFactory& tailFactory);
    };
}
