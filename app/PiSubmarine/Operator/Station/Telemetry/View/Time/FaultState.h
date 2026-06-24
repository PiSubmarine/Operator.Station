#pragma once

#include <QMetaType>

namespace PiSubmarine::Operator::Station::Telemetry::View::Time
{
    enum class FaultState
    {
        Normal = 0,
        Warning = 1,
        Fault = 2
    };
}

Q_DECLARE_METATYPE(PiSubmarine::Operator::Station::Telemetry::View::Time::FaultState)
