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

#include "pch.h"
#include <lightmetrica.test/base.h>
#include <lightmetrica/logger.h>
#include <boost/regex.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace fs = boost::filesystem;

LM_NAMESPACE_BEGIN
LM_TEST_NAMESPACE_BEGIN

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

	Logger::Error("error", "");
	Logger::Warn("warning", "");
	Logger::Info("info", "");
	Logger::Debug("debug", "");

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

	Logger::Error("hello", "");

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
	boost::regex r("\\| ERROR .+ \\| hello");
	EXPECT_TRUE(boost::regex_search(coutSS.str(), r));
	EXPECT_TRUE(boost::regex_search(cerrSS.str(), r));
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
	LM_LOG_INFO("hello");

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
	boost::regex r("^\\| INFO .+ \\| hello");
	EXPECT_TRUE(boost::regex_search(s, r)) << s;

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
			if (entry->level == Logger::LogLevel::Information && boost::algorithm::ends_with(entry->message, "hello"))
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
                LM_LOG_INFO("hello");
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
			if (entry->level == Logger::LogLevel::Information && boost::algorithm::ends_with(entry->message, "hello"))
				emitted = true;
		});

	LM_LOG_INFO("hello");
	EXPECT_TRUE(emitted);
}

LM_TEST_NAMESPACE_END
LM_NAMESPACE_END
