#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include "PiSubmarine/Lease/Api/Identifiers.h"
#include "PiSubmarine/Video/Subscription/Api/Endpoint.h"

namespace PiSubmarine::Operator::Station::Video
{
    struct ReceiveEndpoint
    {
        std::string BindAddress = "0.0.0.0";
        std::uint16_t Port = 5004;

        [[nodiscard]] bool operator==(const ReceiveEndpoint&) const = default;
    };

    struct Config
    {
        ::PiSubmarine::Lease::Api::ResourceId ResourceId{.Value = "video-main"};
        ReceiveEndpoint ReceiveEndpoint{};
        ::PiSubmarine::Video::Subscription::Api::Endpoint SubscriptionEndpoint{
            .Host = "127.0.0.1",
            .Port = 5004};
        std::chrono::milliseconds RetryDelay{1000};

        [[nodiscard]] bool operator==(const Config&) const = default;
    };
}
