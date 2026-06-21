#pragma once

#include <string>

namespace PiSubmarine::Operator::Station::Input
{
    enum class BindingType
    {
        Axis,
        Key
    };

    struct BindingDescriptor
    {
        std::string Name;
        BindingType Type;
    };
}
