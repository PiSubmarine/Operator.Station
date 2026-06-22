#pragma once

#include <vector>

#include <QObject>

#include "PiSubmarine/Control/Api/Input/ISink.h"
#include "PiSubmarine/Operator/Station/Composition/LeaseState.h"
#include "PiSubmarine/Time/ITickable.h"

namespace PiSubmarine::Operator::Station::Composition
{
    class IControl : public QObject, public ::PiSubmarine::Time::ITickable
    {
        Q_OBJECT

    public:
        explicit IControl(QObject* parent = nullptr)
            : QObject(parent)
        {
        }

        virtual ~IControl() = default;

        [[nodiscard]] virtual ::PiSubmarine::Control::Api::Input::ISink& GetSink() = 0;

    signals:
        void LeaseStateChanged(const OptionalLeaseId& leaseId);
    };
}
