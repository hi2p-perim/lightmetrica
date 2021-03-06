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

#pragma once
#ifndef LIB_LIGHTMETRICA_LOGGER_H
#define LIB_LIGHTMETRICA_LOGGER_H

#include "common.h"
#include <string>
#include <memory>
#include <functional>
#include <boost/signals2.hpp>

LM_NAMESPACE_BEGIN

/*!
	Logger.
	Manages log messages.
*/
class LM_PUBLIC_API Logger
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

	LM_DISABLE_COPY_AND_MOVE(Logger);

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
		Clear log entries.
	*/
	static void Clear();

	/*!
		Add an error log message.
		\param message Log message.
		\param prefix Prefix message (inserted before indentation).
	*/
	static void Error(const std::string& message, const std::string& prefix);

	/*!
		Add a warning log message.
		\param message Log message.
		\param prefix Prefix message (inserted before indentation).
	*/
	static void Warn(const std::string& message, const std::string& prefix);

	/*!
		Add an information log message.
		\param message Log message.
		\param prefix Prefix message (inserted before indentation).
	*/
	static void Info(const std::string& message, const std::string& prefix);

	/*!
		Add a debug log message.
		\param message Log message.
		\param prefix Prefix message (inserted before indentation).
	*/
	static void Debug(const std::string& message, const std::string& prefix);

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
		\param line Line number.
		\return Formatted log message.
	*/
	static std::string FormattedDebugInfo(const char* fileName, int line);

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

	/*!
		Get current indentation size.
		\return Indentation.
	*/
	static unsigned int Indentation();

	/*!
		Set the indentation size.
		\param indentation Indentation.
	*/
	static void SetIndentation(unsigned int indentation);

};

/*!
	Log indenter.
	The class automatically increase or decrease an indentation in the current scope.
*/
class LogIndenter
{
public:

	LogIndenter() { Logger::SetIndentation(Logger::Indentation() + 1); }
	~LogIndenter() { Logger::SetIndentation(Logger::Indentation() - 1); }

private:

	LM_DISABLE_COPY_AND_MOVE(LogIndenter);

};

LM_NAMESPACE_END

/*!
	\def LM_LOG_ERROR(message)
	Helper macro to add an error log message.
	\param message Log message.
*/

/*!
	\def LM_LOG_WARN(message)
	Helper macro to add a warning log message.
	\param message Log message.
*/

/*!
	\def LM_LOG_INFO(message)
	Helper macro to add an information log message.
	\param message Log message.
*/

/*!
	\def LM_LOG_DEBUG(message)
	Helper macro to add a debug log message.
	The macro is automatically disabled in the debug mode.
	\param message Log message.
*/

/*!
	\def LM_LOG_INDENT()
	Helper macro to add a indentation to the log message in the same scope.
*/

#if LM_DEBUG_MODE
	#define LM_LOG_ERROR(message) lightmetrica::Logger::Error(message, lightmetrica::Logger::FormattedDebugInfo(__FILE__, __LINE__));
	#define LM_LOG_WARN(message) lightmetrica::Logger::Warn(message, lightmetrica::Logger::FormattedDebugInfo(__FILE__, __LINE__));
	#define LM_LOG_INFO(message) lightmetrica::Logger::Info(message, lightmetrica::Logger::FormattedDebugInfo(__FILE__, __LINE__));
	#define LM_LOG_DEBUG(message) lightmetrica::Logger::Debug(message, lightmetrica::Logger::FormattedDebugInfo(__FILE__, __LINE__));
#else
	#define LM_LOG_ERROR(message) lightmetrica::Logger::Error(message, "");
	#define LM_LOG_WARN(message) lightmetrica::Logger::Warn(message, "");
	#define LM_LOG_INFO(message) lightmetrica::Logger::Info(message, "");
	#define LM_LOG_DEBUG(message) lightmetrica::Logger::Debug(message, "");
#endif
#define LM_LOG_INDENTER() lightmetrica::LogIndenter _logIndenter;

#endif // LIB_LIGHTMETRICA_LOGGER_H
