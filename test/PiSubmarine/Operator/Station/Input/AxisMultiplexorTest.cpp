#include "PiSubmarine/Operator/Station/Input/AxisMultiplexor.h"

#include <chrono>

#include <gtest/gtest.h>

#include "PiSubmarine/Input/Fake/Axis.h"

namespace PiSubmarine::Operator::Station::Input
{
    TEST(AxisMultiplexorTest, ReturnsNeutralValueWhenNoSourcesAreBound)
    {
        AxisMultiplexor multiplexor;

        multiplexor.SetSources({});
        multiplexor.Tick(std::chrono::seconds(0), std::chrono::milliseconds(10));

        EXPECT_DOUBLE_EQ(static_cast<double>(multiplexor.GetValue()), 0.0);
    }

    TEST(AxisMultiplexorTest, LastChangedSourceWinsUntilAnotherSourceChanges)
    {
        double keyboardValue = 0.0;
        double gamepadValue = 0.0;
        ::PiSubmarine::Input::Fake::Axis keyboardAxis([&keyboardValue]()
        {
            return SignedNormalizedFraction(keyboardValue);
        });
        ::PiSubmarine::Input::Fake::Axis gamepadAxis([&gamepadValue]()
        {
            return SignedNormalizedFraction(gamepadValue);
        });
        AxisMultiplexor multiplexor;

        multiplexor.SetSources({&keyboardAxis, &gamepadAxis});

        keyboardValue = 0.25;
        multiplexor.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));
        EXPECT_DOUBLE_EQ(static_cast<double>(multiplexor.GetValue()), 0.25);

        gamepadValue = -0.75;
        multiplexor.Tick(std::chrono::seconds(2), std::chrono::milliseconds(10));
        EXPECT_DOUBLE_EQ(static_cast<double>(multiplexor.GetValue()), -0.75);

        keyboardValue = 0.5;
        multiplexor.Tick(std::chrono::seconds(3), std::chrono::milliseconds(10));
        EXPECT_DOUBLE_EQ(static_cast<double>(multiplexor.GetValue()), 0.5);
    }
}
