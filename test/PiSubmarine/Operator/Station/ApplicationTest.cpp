#include <gtest/gtest.h>

#include "PiSubmarine/Operator/Station/Application.h"

namespace PiSubmarine::Operator::Station
{
    TEST(ApplicationTest, RunReturnsSuccess)
    {
        Application application;

        ASSERT_EQ(application.Run(), 0);
    }
}
