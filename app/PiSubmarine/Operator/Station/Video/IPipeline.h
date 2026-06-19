#pragma once

#include "PiSubmarine/Error/Api/Result.h"

namespace PiSubmarine::Operator::Station::Video
{
    class IPipeline
    {
    public:
        virtual ~IPipeline() = default;

        [[nodiscard]] virtual Error::Api::Result<void> Play() = 0;
        [[nodiscard]] virtual Error::Api::Result<void> Stop() = 0;
        virtual void PollBus() = 0;
        [[nodiscard]] virtual bool IsRunning() const noexcept = 0;
    };
}
