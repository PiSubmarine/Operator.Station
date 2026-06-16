#pragma once

#include <gst/gst.h>

#include "PiSubmarine/Error/Api/Result.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IVideoPipelineTailFactory
    {
    public:
        virtual ~IVideoPipelineTailFactory() = default;

        [[nodiscard]] virtual Error::Api::Result<GstElement*> CreatePipelineTail() = 0;
    };
}
