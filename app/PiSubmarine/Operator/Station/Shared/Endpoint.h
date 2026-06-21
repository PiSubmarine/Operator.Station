#pragma once

#include <cstdint>
#include <string>

namespace PiSubmarine::Operator::Station::Shared
{
    // TODO Is it used anywhere? If not, remove.
    struct Endpoint
    {
        std::string Host = "127.0.0.1";
        std::uint16_t Port = 0;

        [[nodiscard]] bool operator==(const Endpoint&) const = default;
    };
}
