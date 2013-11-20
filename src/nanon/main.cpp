#include <nanon/config.h>
#include <iostream>
#include <exception>

using namespace nanon;

int main(int argc, char** argv)
{
	try
	{
		NanonConfig config;
		
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
#ifdef NANON_DEBUG_MODE
		std::cin.get();
#endif
	}

	return 0;
}
