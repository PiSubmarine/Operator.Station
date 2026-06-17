#pragma once

#include <optional>

#include "PiSubmarine/Lease/Api/Identifiers.h"

namespace PiSubmarine::Operator::Station::Video
{
    // TODO introduce similar Status reporting for Input and Telemetry.
    struct Status
    {
        bool IsStarted = false;
        bool HasTailFactory = false;
        bool HasLease = false;
        bool IsSubscribed = false;
        bool IsPipelineRunning = false;
        std::optional<::PiSubmarine::Lease::Api::LeaseId> LeaseId;

        [[nodiscard]] bool operator==(const Status&) const = default;
    };
}
