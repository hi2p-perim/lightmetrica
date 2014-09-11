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
#ifndef LIGHTMETRICA_PROGRESS_BAR_H
#define LIGHTMETRICA_PROGRESS_BAR_H

#include <lightmetrica/common.h>
#include <string>
#include <mutex>
#include <atomic>
#include <condition_variable>

LM_NAMESPACE_BEGIN

/*!
	Progress bar.
	CLI based progress bar.
*/
class ProgressBar
{
public:

	ProgressBar() {}

private:

	LM_DISABLE_COPY_AND_MOVE(ProgressBar);

public:

	void SetConsoleWidth(int consoleWidth);
	void Begin(const std::string& taskName);
	void End();
	void Abort();
	void OnReportProgress(double progress, bool done);
	void RequestUpdateProgress();
	void ProcessProgressOutput();

private:

	std::atomic<bool> enableProgressBar;
	std::atomic<bool> requiresProgressUpdate;
	bool progressPrintDone;
	bool progressDone;
	double progress;
	std::mutex progressMutex;
	std::string progressTaskName;
	std::condition_variable progressDoneCond;
	void* consoleHandle;
	int consoleWidth;

};

LM_NAMESPACE_END

#endif // LIGHTMETRICA_PROGRESS_BAR_H