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
#include <string>
#include <exception>
#include <thread>
#include <future>
#include <atomic>
#include <chrono>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

using namespace nanon;
namespace po = boost::program_options;

namespace
{

	const std::string AppName = "Nanon Renderer";
	const std::string AppNameShort = "nanon";
	const std::string AppVersion = "0.0.1.dev";
	const std::string AppDesc = AppNameShort + " - " + AppName + " " + AppVersion;

	void PrintHelpMessage(const po::options_description& opt)
	{
		std::cout << AppDesc << std::endl;
		std::cout << std::endl;
		std::cout << "Usage: nanon [arguments] [file ..]" << std::endl;
		std::cout << std::endl;
		std::cout << opt << std::endl;
	}

	bool Run(int argc, char** argv)
	{
		std::string inputFile;

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
				return true;
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
		
		// ----------------------------------------------------------------------

		// Configure the logger
		Logger::SetOutputMode(Logger::LogOutputMode::Stdout | Logger::LogOutputMode::File);

		// Start the logger thread
		std::atomic<bool> logThreadDone = false;
		auto logResult = std::async(
			std::launch::async,
			[&logThreadDone]()
			{
				// Event loop for logger process
				while (!logThreadDone || !Logger::Empty())
				{
					Logger::ProcessOutput();
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
			});

		// ----------------------------------------------------------------------

		// Print start message
		NANON_LOG_INFO("------------------------------------------------------------");
		NANON_LOG_INFO(AppDesc);
		NANON_LOG_INFO("------------------------------------------------------------");
		NANON_LOG_INFO("Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)");
		NANON_LOG_INFO("The software is distributed under the MIT license.");
		NANON_LOG_INFO("For detail see the LICENSE file along with the software.");
		NANON_LOG_INFO("------------------------------------------------------------");

		// ----------------------------------------------------------------------

		// Load input file
		NanonConfig config;
		if (config.Load(inputFile))
		{
			NANON_LOG_DEBUG("");
			return false;
		}

		//RendererDispatcher dispatcher;
		//RendererDispatcher().Dispatch(config);

		// ----------------------------------------------------------------------

		// Wait for the finish of the logger thread
		logThreadDone = true;
		logResult.wait();

		return true;
	}

}

int main(int argc, char** argv)
{
	int result = EXIT_SUCCESS;

	try
	{
		if (!Run(argc, argv))
		{
			result = EXIT_FAILURE;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		result = EXIT_FAILURE;
	}

#ifdef NANON_DEBUG_MODE
	std::cerr << "Press any key to exit ...";
	std::cin.get();
#endif

	return result;
}
