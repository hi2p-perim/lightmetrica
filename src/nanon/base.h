#ifndef __RF_TEST_BASE_H__
#define __RF_TEST_BASE_H__

#include "common.h"
#include <gtest/gtest.h>

RF_TEST_NAMESPACE_BEGIN

class TestBase : public ::testing::Test
{
public:

	// Default timeout in milliseconds
	static const long long OutputProcessTimeout;

protected:

	virtual void SetUp();
	virtual void TearDown();

};

RF_TEST_NAMESPACE_END

#endif // __RF_TEST_BASE_H__