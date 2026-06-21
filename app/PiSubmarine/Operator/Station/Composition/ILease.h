#pragma once

#include <QObject>

#include "PiSubmarine/Lease/Api/ILeaseIssuer.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class ILease
    {
    public:
        virtual ~ILease() = default;

        [[nodiscard]] virtual ::PiSubmarine::Lease::Api::ILeaseIssuer& GetIssuer() = 0;
        [[nodiscard]] virtual QObject& GetWorkerObject() = 0;
    };
}
