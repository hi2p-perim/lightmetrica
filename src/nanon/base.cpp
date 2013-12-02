#include "pch.h"
#include "base.h"
#include <rfcore/logger.h>

using namespace reffect;

RF_TEST_NAMESPACE_BEGIN

const long long TestBase::OutputProcessTimeout = 500;

void TestBase::SetUp()
{
	Logger::Reset();
	Logger::SetOutputMode(Logger::LogOutputMode::Stderr);
	Logger::SetUpdateMode(Logger::LogUpdateMode::Immediate);
}

void TestBase::TearDown()
{
	
}

RF_TEST_NAMESPACE_END