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

#pragma once
#ifndef __NANON_CORE_LOGGER_H__
#define __NANON_CORE_LOGGER_H__

#include "common.h"
#include <string>
#include <memory>
#include <functional>
#include <boost/signals2.hpp>

NANON_NAMESPACE_BEGIN

/*!
	Logger.
	Manages log messages.
*/
class NANON_PUBLIC_API Logger
{
public:

	/*!
		Output mode of the logger.
		Determines the way to output log entries.
		The mode can be selected using binary operators.
	*/
	enum LogOutputMode
	{
		Signal			= 1<<0,		//!< Output to signal LogUpdate.
		Stdout			= 1<<1,		//!< Output to standard output.
		Stderr			= 1<<2,		//!< Output to standard error.
		File			= 1<<3,		//!< Output to external file (in plane text).
		FileHtml		= 1<<4,		//!< Output to external file (in HTML format).
		DebugOutput		= 1<<5,		//!< Output to debug output (only for VC).

		NoFileOutput	= Signal | Stdout | Stderr | DebugOutput,	//!< Specifies the mode with no file output.
		FileOutput		= File | FileHtml,							//!< Specifies the mode with file output.
	};

	/*!
		Update mode of the logger.
		Specifies how to process the log entries.
	*/
	enum class LogUpdateMode
	{
		Manual,			//!< Processes the entries in \a ProcessOutput function.
		Immediate,		//!< Processes the entry immediately. Note that the output mode is limited to \a NoFileOutput.
	};

	/*!
		Log level.
		The log level which associate with the message.
	*/
	enum class LogLevel
	{
		Error,			//!< Error.
		Warning,		//!< Warning.
		Information,	//!< Information.
		Debug,			//!< Debugging (used only for the debug mode).
	};

	/*
		Log entry.
		The entry of the log message.
	*/
	struct LogEntry
	{
		LogLevel level;			//!< Associated log level.
		std::string time;		//!< Current time.
		std::string message;	//!< Log message.
	};

private:

	Logger();
	~Logger();

	NANON_DISABLE_COPY_AND_MOVE(Logger);

public:

	/*!
		Connect to LogUpdate signal.
		The signal is emitted when the output mode is Signal and a log entry is being processed.
		Note that processed entry is disposed.
		\param func Slot function.
	*/
	static boost::signals2::connection Connect_LogUpdate(const std::function<void (LogEntry*)>& func);

public:

	/*!
		Reset the logger to the initial state.
	*/
	static void Reset();

	/*!
		Add an error log message.
		\param message Log message.
	*/
	static void Error(const std::string& message);

	/*!
		Add a warning log message.
		\param message Log message.
	*/
	static void Warn(const std::string& message);

	/*!
		Add an information log message.
		\param message Log message.
	*/
	static void Info(const std::string& message);

	/*!
		Add a debug log message.
		\param message Log message.
	*/
	static void Debug(const std::string& message);

	/*!
		Get number of log entries for the mode NoFileOutput.
		\return Number of log entries.
	*/
	static int CountNoFileOutputEntries();

	/*!
		Get number of log entries for the mode FileOutput.
		\return Number of log entries.
	*/
	static int CountFileOutputEntries();

	/*!
		Set the update mode of the logger.
		Default value is \a Manual, so the function \a ProcessOutput must be called by regular intervals.
		\param mode Update mode.
	*/
	static void SetUpdateMode(LogUpdateMode mode);

	/*!
		Set output mode of the logger.
		In default the log output is redirected to the signal LogUpdate.
		\param mode Output mode.
	*/
	static void SetOutputMode(int mode);

	/*!
		Set output frequency.
		Set to dispatch the output process by \a freq milliseconds.
		Default value is 10.
		\param freq Frequency in milliseconds.
	*/
	static void SetOutputFrequency(int freq);

	/*!
		Set frequency of log output for file output.
		Changes log output frequency by once per \a freq entries when the output mode is \a File or \a FileHtml.
		The frequency should be set relatively slow.
		The default value is 100.
	*/
	static void SetOutputFrequencyForFileOutput(int freq);

	/*!
		Set output file name.
		The file name is used for the mode \a File or \a FileHtml.
		The default value is \a nanon.log.
		\param fileName Output file name.
	*/
	static void SetOutputFileName(const std::string& fileName);

	/*!
		Helper function to output formatted debug message.
		\param fileName File name.
		\param funcName Function name.
		\param line Line number.
		\return Formatted log message.
	*/
	static std::string FormattedDebugInfo(const char* fileName, const char* funcName, int line);

	/*!
		Process logger.
		Dispatches the output process of log entries.
		The function must be called in the event loop.
	*/
	static void ProcessOutput();

	/*!
		Check if the log queue is empty.
		\retval true The queue is empty.
		\retval false The queue is not empty.
	*/ 
	static bool Empty();

};

NANON_NAMESPACE_END

/*!
	\def NANON_LOG_ERROR(message)
	Helper macro to add an error log message.
	\param message Log message.
*/

/*!
	\def NANON_LOG_WARN(message)
	Helper macro to add a warning log message.
	\param message Log message.
*/

/*!
	\def NANON_LOG_INFO(message)
	Helper macro to add an information log message.
	\param message Log message.
*/

/*!
	\def NANON_LOG_DEBUG(message)
	Helper macro to add a debug log message.
	The macro is automatically disabled in the debug mode.
	\param message Log message.
*/

/*!
	\def NANON_LOG_DEBUG_EMPTY()
	Add an debug log with an empty message.
*/

/*!
	\def NANON_LOG_INDENT()
	Helper macro to add a indentation to the log message in the same scope.
*/

#define NANON_LOG_ERROR(message) nanon::Logger::Error(message);
#define NANON_LOG_WARN(message) nanon::Logger::Warn(message);
#define NANON_LOG_INFO(message) nanon::Logger::Info(message);
#ifdef NANON_DEBUG_MODE
	#ifdef NANON_COMPILER_MSVC
		#define NANON_LOG_DEBUG(message) \
			nanon::Logger::Debug(nanon::Logger::FormattedDebugInfo(__FILE__, __FUNCTION__, __LINE__) + message);
	#elif defined(NANON_COMPILER_GCC)
		#define NANON_LOG_DEBUG(message) \
			nanon::Logger::Debug(nanon::Logger::FormattedDebugInfo(__FILE__, __PRETTY_FUNCTION__ , __LINE__) + message);
	#else
		#define NANON_LOG_DEBUG(message)
	#endif
#else
	#define NANON_LOG_DEBUG(message)
#endif
#define NANON_LOG_DEBUG_EMPTY() NANON_LOG_DEBUG("");

#endif // __NANON_CORE_LOGGER_H__
