#pragma once

#include <memory>

#include "PiSubmarine/Operator/Station/Video/IPipelineBuilder.h"
#include "PiSubmarine/Video/Subscription/Api/IService.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class IVideo
    {
    public:
        virtual ~IVideo() = default;

        [[nodiscard]] virtual std::shared_ptr<::PiSubmarine::Operator::Station::Video::IPipelineBuilder> GetPipelineBuilder() = 0;
        [[nodiscard]] virtual ::PiSubmarine::Video::Subscription::Api::IService& GetSubscriptionService() = 0;
    };
}
