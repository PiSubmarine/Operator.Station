#pragma once

#include "PiSubmarine/Operator/Station/IApplication.h"

namespace PiSubmarine::Operator::Station
{
    class Application : public IApplication
    {
    public:
        ~Application() override = default;

        [[nodiscard]] int Run() const override;
    };
}
