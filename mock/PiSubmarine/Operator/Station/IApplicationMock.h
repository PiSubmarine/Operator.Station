#pragma once

#include <gmock/gmock.h>

#include "PiSubmarine/Operator/Station/IApplication.h"

namespace PiSubmarine::Operator::Station
{
    class IApplicationMock : public IApplication
    {
    public:
        MOCK_METHOD(int, Run, (), (const, override));
    };
}
