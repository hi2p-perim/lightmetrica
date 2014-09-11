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

#include "progressbar.h"
#include <iostream>
#include <boost/format.hpp>
#if LM_PLATFORM_WINDOWS
#include <windows.h>
#elif LM_PLATFORM_LINUX
#include <sys/ioctl.h>
#endif

LM_NAMESPACE_BEGIN

void ProgressBar::SetConsoleWidth( int consoleWidth )
{
	this->consoleWidth = consoleWidth;
}

void ProgressBar::Begin( const std::string& taskName )
{
	std::unique_lock<std::mutex> lock(progressMutex);
	progress = 0;
	progressTaskName = taskName;
	progressDone = false;
	requiresProgressUpdate = true;
	progressPrintDone = false;
	enableProgressBar = true;
}

void ProgressBar::End()
{
	std::unique_lock<std::mutex> lock(progressMutex);
	progressDoneCond.wait(lock, [this](){ return progressPrintDone; });
	enableProgressBar = false;
}

void ProgressBar::Abort()
{
	std::unique_lock<std::mutex> lock(progressMutex);
	requiresProgressUpdate = true;
	progressDone = true;
	progressDoneCond.wait(lock, [this](){ return progressPrintDone; });
	enableProgressBar = false;
}

void ProgressBar::OnReportProgress( double progress, bool done )
{
	if (!progressDone)
	{
		std::unique_lock<std::mutex> lock(progressMutex);
		this->progress = progress;
		requiresProgressUpdate = true;
		progressDone = done;
	}
}

void ProgressBar::RequestUpdateProgress()
{
	requiresProgressUpdate = true;
}

void ProgressBar::ProcessProgressOutput()
{
	if (enableProgressBar && requiresProgressUpdate && !progressPrintDone)
	{
		double currentProgress;
		bool currentProgressDone;

		{
			std::unique_lock<std::mutex> lock(progressMutex);
			currentProgress = progress;
			currentProgressDone = progressDone;
			requiresProgressUpdate = false;
		}

		std::string line = boost::str(boost::format("| %s [] %.1f%%") % progressTaskName % (static_cast<double>(currentProgress) * 100.0));
		std::string bar;

		// Bar width
		int barWidth = consoleWidth - static_cast<int>(line.size());
		int p =  static_cast<int>(currentProgress * barWidth);
		for (int j = 0; j < barWidth; j++)
		{
			bar += j <= p ? "=" : " ";
		}

		std::cout << boost::format("| %s [") % progressTaskName;
#if LM_PLATFORM_WINDOWS
		SetConsoleTextAttribute((HANDLE)consoleHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#elif LM_PLATFORM_LINUX
		std::cout << "\033[32m";
#endif
		std::cout << bar;
#if LM_PLATFORM_WINDOWS
		SetConsoleTextAttribute((HANDLE)consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#elif LM_PLATFORM_LINUX
		std::cout << "\033[0m";
#endif
		std::cout << boost::format("] %.1f%%") % (static_cast<double>(currentProgress) * 100.0);

		// If the progress is done, the line is not removed
		if (currentProgressDone)
		{
			std::cout << std::endl;
			progressPrintDone = true;
			progressDoneCond.notify_all();
		}
		else
		{
			std::cout << "\r";
			std::cout.flush();
			progressPrintDone = false;
		}
	}
}

LM_NAMESPACE_END