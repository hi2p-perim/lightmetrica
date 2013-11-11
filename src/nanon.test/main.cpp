#include <iostream>
#include <glm/glm.hpp>
#include <gtest/gtest.h>
#include <nanon/test.h>

TEST(Test1, A)
{
	EXPECT_EQ(100, Func(10));
}
