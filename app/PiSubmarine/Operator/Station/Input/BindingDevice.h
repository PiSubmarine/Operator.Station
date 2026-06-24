#pragma once

#include <QMetaType>

namespace PiSubmarine::Operator::Station::Input
{
    enum class BindingDevice
    {
        Keyboard = 0,
        Gamepad = 1
    };
}

Q_DECLARE_METATYPE(PiSubmarine::Operator::Station::Input::BindingDevice)
