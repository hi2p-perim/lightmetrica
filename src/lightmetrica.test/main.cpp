/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
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
