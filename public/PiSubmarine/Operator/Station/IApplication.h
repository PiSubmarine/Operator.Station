#pragma once

namespace PiSubmarine::Operator::Station
{
    class IApplication
    {
    public:
        virtual ~IApplication() = default;

        [[nodiscard]] virtual int Run() const = 0;
    };
}
