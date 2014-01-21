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

#include "pch.h"
#include <lightmetrica/logger.h>
#ifdef LM_PLATFORM_WINDOWS
#include <Windows.h>
#endif
#include <boost/filesystem.hpp>

namespace bs = boost::signals2;
namespace ch = std::chrono;

namespace
{
	const std::string LogFormat = "| %-5s %s | %s\n";
}

LM_NAMESPACE_BEGIN

class LoggerImpl
{
public:

	static LoggerImpl& Instance()
	{
		static LoggerImpl instance;
		return instance;
	}

public:

	LoggerImpl();

public:

	void SetOutputMode(int mode);
	void AddLogEntry(Logger::LogLevel level, const std::string& message, const std::string& prefix);
	void ProcessOutput();
	void SetOutputFrequency(int freq);
	void SetOutputFrequencyForFileOutput(int freq);
	int CountNoFileOutputEntries();
	int CountFileOutputEntries();
	void Reset();
	void SetOutputFileName(const std::string& fileName);
	void SetUpdateMode(Logger::LogUpdateMode mode);
	bool Empty();
	unsigned int Indentation() { return indentation; }
	void SetIndentation(unsigned int indentation);

private:

	std::string GetLevelString(Logger::LogLevel level);
	void ProcessSingleEntryForNoFileOutput(const std::shared_ptr<Logger::LogEntry>& entry);

private:

	std::deque<std::shared_ptr<Logger::LogEntry>> entries;				// Entries for the mode Signal, Stdout, Stderr, or DebugOutput
	std::deque<std::shared_ptr<Logger::LogEntry>> entriesForFileIO;		// Entries for the mode File or FileHtml
	std::mutex mutex;
	ch::high_resolution_clock::time_point begin;
	ch::high_resolution_clock::time_point lastOutputTime;				// For the mode Signal, Stdout, Stderr, or DebugOutput
	ch::high_resolution_clock::time_point lastOutputTimeForFileIO;		// For the mode File or FileHtml
	int outputFrequency;
	int outputFrequencyForFileIO;
	int outputMode;
	std::string outputFileName;
	Logger::LogUpdateMode updateMode;
	unsigned int indentation;
	std::string indentationStr;

public:

	bs::signal<void (Logger::LogEntry*)> signal_LogUpdate;

};

LoggerImpl::LoggerImpl()
{
	Reset();
}

void LoggerImpl::AddLogEntry( Logger::LogLevel level, const std::string& message, const std::string& prefix )
{
	// Current time
	auto now = ch::high_resolution_clock::now();
	double elapsed = ch::duration_cast<ch::milliseconds>(now - begin).count() / 1000.0;

	// Record the entry
	{
		std::unique_lock<std::mutex> lock(mutex);
		auto entry = std::make_shared<Logger::LogEntry>();
		entry->level = level;
		entry->time = boost::str(boost::format("%.3f") % elapsed);
		entry->message = (prefix.empty() ? "" : prefix + " ") + indentationStr + message;

		if (updateMode == Logger::LogUpdateMode::Manual)
		{
			if ((outputMode & Logger::LogOutputMode::NoFileOutput) > 0)
			{
				entries.push_back(entry);
			}

			if ((outputMode & Logger::LogOutputMode::FileOutput) > 0)
			{
				entriesForFileIO.push_back(entry);
			}
		}
		else if (updateMode == Logger::LogUpdateMode::Immediate)
		{
			// In the immediate mode, process the entry immediately
			if ((outputMode & Logger::LogOutputMode::NoFileOutput) > 0)
			{
				if ((outputMode & Logger::LogOutputMode::Signal) > 0)
				{
					signal_LogUpdate(entry.get());
				}

				if ((outputMode & Logger::LogOutputMode::Stdout) > 0 ||
					(outputMode & Logger::LogOutputMode::Stderr) > 0 ||
					(outputMode & Logger::LogOutputMode::DebugOutput) > 0 ||
					(outputMode & Logger::LogOutputMode::File) > 0)
				{
					ProcessSingleEntryForNoFileOutput(entry);
				}
			}
		}
	}
}

void LoggerImpl::SetOutputMode( int mode )
{
	outputMode = mode;
}

void LoggerImpl::ProcessOutput()
{
	if (updateMode != Logger::LogUpdateMode::Manual)
	{
		return;
	}

	// Process the mode Signal, Stdout, Stderr, or DebugOutput
	if ((outputMode & Logger::LogOutputMode::NoFileOutput) > 0)
	{
		auto now = ch::high_resolution_clock::now();
		long long elapsed = ch::duration_cast<ch::milliseconds>(now - lastOutputTime).count();

		if (elapsed > outputFrequency)
		{
			std::unique_lock<std::mutex> lock(mutex);

			if (!entries.empty())
			{
				if ((outputMode & Logger::LogOutputMode::Signal) > 0)
				{
					for (auto& entry : entries)
					{
						signal_LogUpdate(entry.get());
					}
				}

				if ((outputMode & Logger::LogOutputMode::Stdout) > 0 ||
					(outputMode & Logger::LogOutputMode::Stderr) > 0 ||
					(outputMode & Logger::LogOutputMode::DebugOutput) > 0 ||
					(outputMode & Logger::LogOutputMode::File) > 0)
				{
					for (auto& entry : entries)
					{
						ProcessSingleEntryForNoFileOutput(entry);
					}
				}
			}

			entries.clear();
			lastOutputTime = now;
		}
	}

	if ((outputMode & Logger::LogOutputMode::FileOutput) > 0)
	{
		// Process the mode File or FileHtml
		auto now = ch::high_resolution_clock::now();
		long long elapsed = ch::duration_cast<ch::milliseconds>(now - lastOutputTimeForFileIO).count();

		// Open file
		std::ofstream ofs(outputFileName.c_str(), std::ios::out | std::ios::app);

		if (elapsed > outputFrequencyForFileIO)
		{
			std::unique_lock<std::mutex> lock(mutex);

			if (!entriesForFileIO.empty())
			{
				for (auto& entry : entriesForFileIO)
				{
					auto levelStr = GetLevelString(entry->level);
					auto line = boost::str(boost::format(LogFormat) % levelStr % entry->time % entry->message);
					ofs << line;
				}
			}

			entriesForFileIO.clear();
			lastOutputTimeForFileIO = now;
		}
	}
}

void LoggerImpl::ProcessSingleEntryForNoFileOutput( const std::shared_ptr<Logger::LogEntry>& entry )
{
	auto levelStr = GetLevelString(entry->level);
	auto line = boost::str(boost::format(LogFormat) % levelStr % entry->time % entry->message);

	if ((outputMode & Logger::LogOutputMode::Stdout) > 0)
	{
#ifdef LM_PLATFORM_WINDOWS
		HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		WORD colorFlag = 0;
		if (entry->level == Logger::LogLevel::Error)
			colorFlag = FOREGROUND_RED | FOREGROUND_INTENSITY;
		else if (entry->level == Logger::LogLevel::Warning)
			colorFlag = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		else if (entry->level == Logger::LogLevel::Debug)
			colorFlag = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		else if (entry->level == Logger::LogLevel::Information)
			colorFlag = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
		SetConsoleTextAttribute(consoleHandle, colorFlag);
#elif defined(LM_PLATFORM_LINUX)
		if (entry->level == Logger::LogLevel::Error)
			std::cout << "\033[31m";
		else if (entry->level == Logger::LogLevel::Warning)
			std::cout << "\033[33m";
		else if (entry->level == Logger::LogLevel::Debug)
			std::cout << "\033[37m";
		else if (entry->level == Logger::LogLevel::Information)
			std::cout << "\033[137m";
#endif

		std::cout << line;

#ifdef LM_PLATFORM_WINDOWS
		SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#elif defined(LM_PLATFORM_LINUX)
		std::cout << "\033[0m";
#endif
	}

	if ((outputMode & Logger::LogOutputMode::Stderr) > 0)
	{
		std::cerr << line;
	}

#if defined(LM_DEBUG_MODE) && defined(LM_PLATFORM_WINDOWS)
	if ((outputMode & Logger::LogOutputMode::DebugOutput) > 0)
	{
		OutputDebugStringA(line.c_str());
	}
#endif
}

void LoggerImpl::SetOutputFrequency( int freq )
{
	outputFrequency = freq;
}

void LoggerImpl::SetOutputFrequencyForFileOutput( int freq )
{
	outputFrequencyForFileIO = freq;
}

std::string LoggerImpl::GetLevelString( Logger::LogLevel level )
{
	static const std::string logLevelString[] =
	{
		"ERROR",
		"WARN",
		"INFO",
		"DEBUG"
	};

	return logLevelString[static_cast<int>(level)];
}

int LoggerImpl::CountNoFileOutputEntries()
{
	std::unique_lock<std::mutex> lock(mutex);
	return static_cast<int>(entries.size());
}

int LoggerImpl::CountFileOutputEntries()
{
	std::unique_lock<std::mutex> lock(mutex);
	return static_cast<int>(entriesForFileIO.size());
}

void LoggerImpl::Reset()
{
	std::unique_lock<std::mutex> lock(mutex);
	entries.clear();
	entriesForFileIO.clear();
	outputMode = Logger::LogOutputMode::Signal;
	outputFrequency = 10;
	outputFrequencyForFileIO = 100;
	begin = lastOutputTime = ch::high_resolution_clock::now();
	outputFileName = "nanon.log";
	updateMode = Logger::LogUpdateMode::Manual;
	indentation = 0;
	indentationStr = "";
    signal_LogUpdate.disconnect_all_slots();
}

void LoggerImpl::SetOutputFileName( const std::string& fileName )
{
	outputFileName = fileName;
}

void LoggerImpl::SetUpdateMode( Logger::LogUpdateMode mode )
{
	updateMode = mode;
}

bool LoggerImpl::Empty()
{
	std::unique_lock<std::mutex> lock(mutex);
	return entries.empty() && entriesForFileIO.empty();
}

void LoggerImpl::SetIndentation( unsigned int indentation )
{
	this->indentation = indentation;

	// Update indentation string
	if (indentation > 0)
	{
		indentationStr = std::string(4 * indentation, '.') + " ";
	}
	else
	{
		indentationStr = "";
	}

}

// --------------------------------------------------------------------------------

boost::signals2::connection Logger::Connect_LogUpdate( const std::function<void (LogEntry*)>& func )
{
	auto& p = LoggerImpl::Instance();
	return p.signal_LogUpdate.connect(func);
}

void Logger::Error( const std::string& message, const std::string& prefix )
{
	auto& p = LoggerImpl::Instance();
	p.AddLogEntry(LogLevel::Error, message, prefix);
}

void Logger::Warn( const std::string& message, const std::string& prefix )
{
	auto& p = LoggerImpl::Instance();
	p.AddLogEntry(LogLevel::Warning, message, prefix);
}

void Logger::Info( const std::string& message, const std::string& prefix )
{
	auto& p = LoggerImpl::Instance();
	p.AddLogEntry(LogLevel::Information, message, prefix);
}

void Logger::Debug( const std::string& message, const std::string& prefix )
{
	auto& p = LoggerImpl::Instance();
	p.AddLogEntry(LogLevel::Debug, message, prefix);
}

std::string Logger::FormattedDebugInfo( const char* fileName, int line )
{
	namespace fs = boost::filesystem;
	auto file = fs::path(fileName).filename().string();
	return boost::str(boost::format("%-5.5s~@%4d |") % file % line);
}

void Logger::SetOutputMode( int mode )
{
	auto& p = LoggerImpl::Instance();
	p.SetOutputMode(mode);
}

void Logger::ProcessOutput()
{
	auto& p = LoggerImpl::Instance();
	p.ProcessOutput();
}

void Logger::SetOutputFrequency( int freq )
{
	auto& p = LoggerImpl::Instance();
	p.SetOutputFrequency(freq);
}

int Logger::CountNoFileOutputEntries()
{
	auto& p = LoggerImpl::Instance();
	return p.CountNoFileOutputEntries();
}

int Logger::CountFileOutputEntries()
{
	auto& p = LoggerImpl::Instance();
	return p.CountFileOutputEntries();
}

void Logger::Reset()
{
	auto& p = LoggerImpl::Instance();
	return p.Reset();
}

void Logger::SetOutputFrequencyForFileOutput( int freq )
{
	auto& p = LoggerImpl::Instance();
	p.SetOutputFrequencyForFileOutput(freq);
}

void Logger::SetOutputFileName( const std::string& fileName )
{
	auto& p = LoggerImpl::Instance();
	p.SetOutputFileName(fileName);
}

void Logger::SetUpdateMode( LogUpdateMode mode )
{
	auto& p = LoggerImpl::Instance();
	p.SetUpdateMode(mode);
}

bool Logger::Empty()
{
	auto& p = LoggerImpl::Instance();
	return p.Empty();
}

unsigned int Logger::Indentation()
{
	auto& p = LoggerImpl::Instance();
	return p.Indentation();
}

void Logger::SetIndentation( unsigned int indentation )
{
	auto& p = LoggerImpl::Instance();
	p.SetIndentation(indentation);
}

LM_NAMESPACE_END
