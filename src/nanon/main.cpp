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

#include <nanon/config.h>
#include <nanon/rendererdispatcher.h>
#include <nanon/logger.h>
#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

using namespace nanon;
namespace po = boost::program_options;

namespace
{
	const std::string AppName = "Nanon Renderer";
	const std::string AppNameShort = "nanon";
	const std::string AppVersion = "0.0.1.dev";
	const std::string AppDescription = AppNameShort + " - " + AppName + " " + AppVersion;
}

class NanonApplication
{
public:

	NanonApplication();

public:

	bool ParseArguments(int argc, char** argv);
	bool Run();
	void StartLogging();
	void FinishLogging();

private:

	void PrintHelpMessage(const po::options_description& opt);
	void PrintStartMessage();
	void PrintCurrentTime();

private:

	// Command line parameters
	std::string inputFile;

	// Logging thread related variables
	std::atomic<bool> logThreadDone;
	std::future<void> logResult;

};

NanonApplication::NanonApplication()
	: logThreadDone(false)
{

}

void NanonApplication::PrintHelpMessage( const po::options_description& opt )
{
	std::cout << AppDescription << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: nanon [arguments] [file ..]" << std::endl;
	std::cout << std::endl;
	std::cout << opt << std::endl;
}

bool NanonApplication::ParseArguments( int argc, char** argv )
{
	// Define options
	po::options_description opt("Allowed options");
	opt.add_options()
		("help", "Display help message")
		("input-file,i", po::value<std::string>(&inputFile)->required(), "Input file (*.nanon)");

	// All positional position are translated into --nanon-file
	po::positional_options_description p;
	p.add("input-file", -1);

	po::variables_map vm;

	try
	{
		// Parse options
		po::store(po::command_line_parser(argc, argv).options(opt).positional(p).run(), vm);

		if (vm.count("help") || argc == 1)
		{
			PrintHelpMessage(opt);
			return false;
		}

		po::notify(vm);
	}
	catch (po::required_option& e)
	{
		// Some options are missing
		std::cout << "ERROR : " << e.what() << std::endl;
		PrintHelpMessage(opt);
		return false;
	}
	catch (po::error& e)
	{
		// Error on parsing options
		std::cout << "ERROR : " << e.what() << std::endl;
		PrintHelpMessage(opt);
		return false;
	}

	return true;
}

bool NanonApplication::Run()
{
	PrintStartMessage();
	PrintCurrentTime();

	// Load input file
	NanonConfig config;
	if (!config.Load(inputFile))
	{
		NANON_LOG_DEBUG("");
		return false;
	}

	//RendererDispatcher dispatcher;
	//RendererDispatcher().Dispatch(config);

	return true;
}

void NanonApplication::StartLogging()
{
	// Configure the logger
	Logger::SetOutputMode(Logger::LogOutputMode::Stdout | Logger::LogOutputMode::File);

	// Start the logger thread
	logResult = std::async(
		std::launch::async,
		[this]()
		{
			// Event loop for logger process
			while (!logThreadDone || !Logger::Empty())
			{
				Logger::ProcessOutput();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		});
}

void NanonApplication::FinishLogging()
{
	// Wait for the finish of the logger thread
	logThreadDone = true;
	logResult.wait();
}

void NanonApplication::PrintStartMessage()
{
	NANON_LOG_INFO("------------------------------------------------------------");
	NANON_LOG_INFO(AppDescription);
	NANON_LOG_INFO("------------------------------------------------------------");
	NANON_LOG_INFO("Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)");
	NANON_LOG_INFO("The software is distributed under the MIT license.");
	NANON_LOG_INFO("For detail see the LICENSE file along with the software.");
	NANON_LOG_INFO("------------------------------------------------------------");
}

void NanonApplication::PrintCurrentTime()
{
	// Current time
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
#ifdef NANON_PLATFORM_WINDOWS
	struct tm timeinfo;
	localtime_s(&timeinfo, &time);
	ss << std::put_time(&timeinfo, "%Y.%m.%d.%H.%M.%S");
#else
	ss << std::put_time(std::localtime(&time), "%Y.%m.%d.%H.%M.%S");
#endif
	NANON_LOG_INFO("CURRENT TIME : " + ss.str());
}

int main(int argc, char** argv)
{
	int result = EXIT_SUCCESS;
	NanonApplication app;

	if (app.ParseArguments(argc, argv))
	{
		app.StartLogging();

		try
		{
			if (!app.Run())
			{
				result = EXIT_FAILURE;
			}
		}
		catch (const std::exception& e)
		{
			NANON_LOG_ERROR(boost::str(boost::format("[ EXCEPTION ] %s") % e.what()));
			result = EXIT_FAILURE;
		}

		app.FinishLogging();
	}

#ifdef NANON_DEBUG_MODE
	std::cerr << "Press any key to exit ...";
	std::cin.get();
#endif

	return result;
}
