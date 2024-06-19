#include <iostream>

#include "gtest/gtest.h"

using namespace std;

TEST(GOOGLETEST, TEST)
{
    int i = 1;
    EXPECT_EQ(i, 1);
}
