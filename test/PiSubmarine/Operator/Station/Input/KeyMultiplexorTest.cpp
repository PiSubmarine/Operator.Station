#include "PiSubmarine/Operator/Station/Input/KeyMultiplexor.h"

#include <chrono>

#include <gtest/gtest.h>

#include "PiSubmarine/Input/Fake/Key.h"

namespace PiSubmarine::Operator::Station::Input
{
    TEST(KeyMultiplexorTest, ReturnsReleasedStateWhenNoSourcesAreBound)
    {
        KeyMultiplexor multiplexor;

        multiplexor.SetSources({});
        multiplexor.Tick(std::chrono::seconds(0), std::chrono::milliseconds(10));

        EXPECT_EQ(multiplexor.GetState(), ::PiSubmarine::Input::Api::KeyState::Released);
    }

    TEST(KeyMultiplexorTest, LastChangedSourceWinsUntilAnotherSourceChanges)
    {
        auto keyboardState = ::PiSubmarine::Input::Api::KeyState::Released;
        auto gamepadState = ::PiSubmarine::Input::Api::KeyState::Released;
        ::PiSubmarine::Input::Fake::Key keyboardKey([&keyboardState]()
        {
            return keyboardState;
        });
        ::PiSubmarine::Input::Fake::Key gamepadKey([&gamepadState]()
        {
            return gamepadState;
        });
        KeyMultiplexor multiplexor;

        multiplexor.SetSources({&keyboardKey, &gamepadKey});

        keyboardState = ::PiSubmarine::Input::Api::KeyState::Pressed;
        multiplexor.Tick(std::chrono::seconds(1), std::chrono::milliseconds(10));
        EXPECT_EQ(multiplexor.GetState(), ::PiSubmarine::Input::Api::KeyState::Pressed);

        gamepadState = ::PiSubmarine::Input::Api::KeyState::Pressed;
        multiplexor.Tick(std::chrono::seconds(2), std::chrono::milliseconds(10));
        EXPECT_EQ(multiplexor.GetState(), ::PiSubmarine::Input::Api::KeyState::Pressed);

        keyboardState = ::PiSubmarine::Input::Api::KeyState::Released;
        multiplexor.Tick(std::chrono::seconds(3), std::chrono::milliseconds(10));
        EXPECT_EQ(multiplexor.GetState(), ::PiSubmarine::Input::Api::KeyState::Released);
    }
}
