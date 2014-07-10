/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/fp.h>

using namespace testing;
using namespace lightmetrica;

class ProxyTestEventListener : public TestEventListener
{
public:

	ProxyTestEventListener(TestEventListener* listener)
		: TestEventListener()
		, listener(listener)
	{

	}

public:

	virtual void OnTestProgramStart( const UnitTest& unit_test )					{ listener->OnTestProgramStart(unit_test); }
	virtual void OnTestIterationStart( const UnitTest& unit_test, int iteration )	{ listener->OnTestIterationStart(unit_test, iteration); }
	virtual void OnEnvironmentsSetUpStart( const UnitTest& unit_test )				{ /*listener->OnEnvironmentsSetUpStart(unit_test);*/ }
	virtual void OnEnvironmentsSetUpEnd( const UnitTest& unit_test )				{ /*listener->OnEnvironmentsSetUpEnd(unit_test);*/ }
	virtual void OnTestCaseStart( const TestCase& test_case )						{ /*listener->OnTestCaseStart(test_case);*/ }

	virtual void OnTestStart( const TestInfo& test_info )
	{
		// Reset logger
		Logger::Reset();
		Logger::SetOutputMode(Logger::Stdout);
		Logger::SetUpdateMode(Logger::LogUpdateMode::Manual);
		Logger::SetOutputFrequency(-1);

		listener->OnTestStart(test_info);
	}

	virtual void OnTestPartResult( const TestPartResult& test_part_result )
	{
		if (test_part_result.failed())
		{
			// Print logs if the test failed
			Logger::ProcessOutput();
		}
		else
		{
			// Otherwise clear log entries
			Logger::Clear();
		}

		listener->OnTestPartResult(test_part_result);
	}

	virtual void OnTestEnd( const TestInfo& test_info )
	{
		if (test_info.result()->Failed())
		{
			Logger::ProcessOutput();
			listener->OnTestEnd(test_info);
		}
	}

	virtual void OnTestCaseEnd( const TestCase& test_case )							{ /*listener->OnTestCaseEnd(test_case);*/ }
	virtual void OnEnvironmentsTearDownStart( const UnitTest& unit_test )			{ /*listener->OnEnvironmentsTearDownStart(unit_test);*/ }
	virtual void OnEnvironmentsTearDownEnd( const UnitTest& unit_test )				{ /*listener->OnEnvironmentsTearDownEnd(unit_test);*/ }
	virtual void OnTestIterationEnd( const UnitTest& unit_test, int iteration )		{ listener->OnTestIterationEnd(unit_test, iteration); }
	virtual void OnTestProgramEnd( const UnitTest& unit_test )						{ listener->OnTestProgramEnd(unit_test); }

private:

	TestEventListener* listener;

};

int main(int argc, char** argv)
{
	// Initialize google test
	testing::InitGoogleTest(&argc, argv);

	// Replace default printer
	auto& listeners = testing::UnitTest::GetInstance()->listeners();
	auto defaultPriner = listeners.Release(listeners.default_result_printer());
	listeners.Append(new ProxyTestEventListener(defaultPriner));

	// Floating-point control
#if LM_STRICT_FP && LM_PLATFORM_WINDOWS
	FloatintPointUtils::EnableFPControl();
#endif

	return RUN_ALL_TESTS();
}
