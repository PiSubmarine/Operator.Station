#pragma once

#include <optional>

#include <QObject>
#include <QString>

namespace PiSubmarine::Operator::Station::Composition
{
    using OptionalLeaseId = std::optional<QString>;
}

Q_DECLARE_METATYPE(PiSubmarine::Operator::Station::Composition::OptionalLeaseId)
