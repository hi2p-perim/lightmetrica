/*
	nanon : A research-oriented renderer

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

#include "pch.h"
#include "base.h"
#include <nanon/logger.h>
#include <regex>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace nanon;
namespace fs = boost::filesystem;

NANON_TEST_NAMESPACE_BEGIN

class LoggerTest : public TestBase {};

TEST_F(LoggerTest, LogMessagesWithVariousLevels)
{
	Logger::Reset();

	bool emitted_error = false;
	bool emitted_warn = false;
	bool emitted_info = false;
	bool emitted_debug = false;

	Logger::Connect_LogUpdate(
		[&](Logger::LogEntry* entry)
		{
			if (entry->level == Logger::LogLevel::Error && entry->message == "error")
				emitted_error = true;
			if (entry->level == Logger::LogLevel::Warning && entry->message == "warning")
				emitted_warn = true;
			if (entry->level == Logger::LogLevel::Information && entry->message == "info")
				emitted_info = true;
			if (entry->level == Logger::LogLevel::Debug && entry->message == "debug")
				emitted_debug = true;
		});

	Logger::Error("error");
	Logger::Warn("warning");
	Logger::Info("info");
	Logger::Debug("debug");

	long long elapsed = 0;
	auto start = std::chrono::high_resolution_clock::now();
	while (Logger::CountNoFileOutputEntries() > 0 && elapsed < OutputProcessTimeout)
	{
		Logger::ProcessOutput();
		auto now = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	}

	EXPECT_TRUE(elapsed < OutputProcessTimeout);
	EXPECT_TRUE(emitted_error);
	EXPECT_TRUE(emitted_warn);
	EXPECT_TRUE(emitted_info);
	EXPECT_TRUE(emitted_debug);
}

TEST_F(LoggerTest, OutputToStdoutOrStderr)
{
	Logger::Reset();
	Logger::SetOutputMode(Logger::LogOutputMode::Stdout | Logger::LogOutputMode::Stderr);

	std::stringstream coutSS, cerrSS;
	std::streambuf* coutBufferOld = std::cout.rdbuf(coutSS.rdbuf());
	std::streambuf* cerrBufferOld = std::cerr.rdbuf(cerrSS.rdbuf());

	Logger::Error("hello");

	long long elapsed = 0;
	auto start = std::chrono::high_resolution_clock::now();
	while (Logger::CountNoFileOutputEntries() > 0 && elapsed < OutputProcessTimeout)
	{
		Logger::ProcessOutput();
		auto now = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	}

	std::cout.rdbuf(coutBufferOld);
	std::cerr.rdbuf(cerrBufferOld);

	EXPECT_TRUE(elapsed < OutputProcessTimeout);

	// Check if output messages contain 'hello'
	std::regex r("^\\[ Error .+ \\] hello");
	EXPECT_TRUE(std::regex_search(coutSS.str(), r));
	EXPECT_TRUE(std::regex_search(cerrSS.str(), r));
}

TEST_F(LoggerTest, OutputToFile)
{
	Logger::Reset();
	Logger::SetOutputMode(Logger::LogOutputMode::File);

	// Use temporary path for output directory
	const std::string filename = (fs::temp_directory_path() / "reffect.test.log").string();

	// Remove file if exists
	if (fs::exists(filename))
	{
		EXPECT_TRUE(fs::remove(filename));
	}

	Logger::SetOutputFileName(filename);
	NANON_LOG_INFO("hello");
	
	long long elapsed = 0;
	auto start = std::chrono::high_resolution_clock::now();
	while (Logger::CountFileOutputEntries() > 0 && elapsed < OutputProcessTimeout)
	{
		Logger::ProcessOutput();
		auto now = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	}

	EXPECT_TRUE(elapsed < OutputProcessTimeout);

	// Open file and check contents
	std::ifstream ifs(filename);
	EXPECT_TRUE(ifs.is_open());

	std::string s;
	std::getline(ifs, s);
	std::regex r("^\\[ Information .+ \\] hello");
	EXPECT_TRUE(std::regex_search(s, r)) << s;

	// Clean up
	ifs.close();
	EXPECT_TRUE(fs::remove(filename));
}

TEST_F(LoggerTest, AddLogFromAnotherThread)
{
	Logger::Reset();

	int count = 0;
	const int maxCount = 100;
	Logger::Connect_LogUpdate(
		[&](Logger::LogEntry* entry)
		{
			if (entry->level == Logger::LogLevel::Information && entry->message == "hello")
				count++;
		});

	// Create thread and add a log entry from the thread
	std::mutex mutex;
	std::condition_variable cv;
	std::thread thread(
		[&]()
		{
			// After some time, a log entry is added
			std::unique_lock<std::mutex> lock(mutex);
			cv.wait(lock);
			for (int i = 0; i < maxCount; i++)
				NANON_LOG_INFO("hello");
		});

	// Simulates event loop
	bool added = false;
	long long elapsed = 0;
	auto start = std::chrono::high_resolution_clock::now();
	while (!added || (count < maxCount && elapsed < OutputProcessTimeout))
	{
		Logger::ProcessOutput();

		// Notify to add an log message from another thread
		if (elapsed > 50 && !added)
		{
			added = true;
			cv.notify_one();
		}

		auto now = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
	}

	thread.join();

	EXPECT_TRUE(elapsed < OutputProcessTimeout);
	EXPECT_EQ(maxCount, count);
}

TEST_F(LoggerTest, ImmediateMode)
{
	Logger::Reset();
	Logger::SetUpdateMode(Logger::LogUpdateMode::Immediate);

	bool emitted = false;
	Logger::Connect_LogUpdate(
		[&](Logger::LogEntry* entry)
	{
		if (entry->level == Logger::LogLevel::Information && entry->message == "hello")
			emitted = true;
	});

	NANON_LOG_INFO("hello");
	EXPECT_TRUE(emitted);
}

NANON_TEST_NAMESPACE_END