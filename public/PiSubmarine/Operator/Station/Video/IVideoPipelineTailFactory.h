#pragma once

#include <gst/gst.h>

#include "PiSubmarine/Error/Api/Result.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IVideoPipelineTailFactory
    {
    public:
        virtual ~IVideoPipelineTailFactory() = default;

        // The returned element must accept video on its "sink" pad directly or
        // be a bin exposing a ghost "sink" pad.
        [[nodiscard]] virtual Error::Api::Result<GstElement*> CreatePipelineTail() = 0;
    };
}
