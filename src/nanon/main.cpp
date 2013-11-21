#include <nanon/config.h>
#include <nanon/rendererdispatcher.h>
#include <iostream>
#include <string>
#include <exception>
#include <boost/program_options.hpp>

using namespace nanon;

namespace
{

	const std::string appName = "nanon";

	bool Run(int argc, char** argv)
	{
		namespace po = boost::program_options;

		// Define options
		po::option_description opt(appName.c_str());
		

		NanonConfig config;
		if (config.Load(argc, argv))
		{
			
		}

		RendererDispatcher dispatcher;



		RendererDispatcher().Dispatch(config);
	}

}

int main(int argc, char** argv)
{
	try
	{
		if (!Run(argc, argv))
		{
			return 1;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
#ifdef NANON_DEBUG_MODE
		std::cin.get();
#endif
	}

	// TODO : Dump log

	return 0;
}

