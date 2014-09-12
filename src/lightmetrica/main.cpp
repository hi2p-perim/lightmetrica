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
#include <lightmetrica/config.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/assets.h>
#include <lightmetrica/version.h>
#include <lightmetrica/primitives.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/renderer.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/light.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/texture.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/math.h>
#include <lightmetrica/fp.h>
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
#if LM_PLATFORM_WINDOWS
#include <windows.h>
#elif LM_PLATFORM_LINUX
#include <sys/ioctl.h>
#endif
#if LM_MPI
#include <mpi.h>
#endif

using namespace lightmetrica;
namespace po = boost::program_options;

class LightmetricaApplication
{
public:

	LightmetricaApplication();

public:

	bool ParseArguments(int argc, char** argv);
	bool Initialize(int argc, char** argv);
	bool Run();
	void StartLogging();
	void FinishLogging();

private:

	bool LoadConfiguration(Config& config);
	bool LoadAssets(const Config& config, Assets& assets);
	bool LoadAndBuildScene(const Config& config, const Assets& assets, Scene& scene);
	bool ConfigureAndDispatchRenderer(const Config& config, const Assets& assets, const Scene& scene, Renderer& renderer);

private:

	void SetAppInfo();
	void PrintHelpMessage(const po::options_description& opt);
	void PrintStartMessage();
	void PrintFinishMessage();
	std::string CurrentTime();

public:

#if LM_PLATFORM_WINDOWS
	static void SETransFunc(unsigned int code, PEXCEPTION_POINTERS data);
#endif

private:

	// Application info
	std::string appName;
	std::string appDescription;
	std::string appFlags;

	// Command line parameters
	std::string inputFile;
	std::string outputImagePath;
	bool interactiveMode;
	std::string basePath;
	double terminationTime;
	bool mpiMode;

	// Logging thread related variables
	std::atomic<bool> logThreadDone;
	std::future<void> logResult;

	// Progress bar
	bool useProgressBar;
	ProgressBar progressBar;

	// For MPI
	int rank;

};

LightmetricaApplication::LightmetricaApplication()
	: logThreadDone(false)
{
	SetAppInfo();
}

void LightmetricaApplication::SetAppInfo()
{
	appName = "Lightmetrica";
	appDescription = boost::str(boost::format("%s Version %s (%s)") % appName % Version::Formatted() % Version::Codename());

	// Enumerate flags
	appFlags = "";
	appFlags += LM_SINGLE_PRECISION	? "single_precision " : "";
	appFlags += LM_DOUBLE_PRECISION	? "double_precision " : "";
	appFlags += LM_MULTI_PRECISION	? "multi_precision "  : "";
	appFlags += LM_SSE    ? "sse "    : "";
	appFlags += LM_SSE2   ? "sse2 "   : "";
	appFlags += LM_SSE3   ? "sse3 "   : "";
	appFlags += LM_SSSE3  ? "ssse3 "  : "";
	appFlags += LM_SSE4_1 ? "sse4.1 " : "";
	appFlags += LM_SSE4_2 ? "sse4.2 " : "";
	appFlags += LM_SSE4A  ? "sse4a "  : "";
	appFlags += LM_AVX    ? "avx "    : "";
}

void LightmetricaApplication::PrintHelpMessage( const po::options_description& opt )
{
	std::cout << appDescription << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: lightmetrica [arguments] [file ..]" << std::endl;
	std::cout << std::endl;
	std::cout << opt << std::endl;
}

bool LightmetricaApplication::ParseArguments( int argc, char** argv )
{
	// Define options
	po::options_description opt("Allowed options");
	opt.add_options()
		("help", "Display help message")
		("config,f", po::value<std::string>(&inputFile), "Configuration file")
		("output-image,o", po::value<std::string>(&outputImagePath)->default_value(""), "Output image path")
		("interactive,i", po::bool_switch(&interactiveMode), "Interactive mode")
		("base-path,b", po::value<std::string>(&basePath)->default_value(""), "Base path for asset loading")
		("termination-time,t", po::value<double>(&terminationTime)->default_value(0), "Termination time for rendering")
		("mpi", po::bool_switch(&mpiMode), "MPI mode");

	// positional arguments
	po::positional_options_description p;
	p.add("config", 1);
	p.add("output-image", 2);

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
		std::cerr << "ERROR : " << e.what() << std::endl;
		PrintHelpMessage(opt);
		return false;
	}
	catch (po::error& e)
	{
		// Error on parsing options
		std::cerr << "ERROR : " << e.what() << std::endl;
		PrintHelpMessage(opt);
		return false;
	}

	if (vm.count("config") && interactiveMode)
	{
		std::cerr << "Conflicting arguments : 'config' and 'interactive'" << std::endl;
		PrintHelpMessage(opt);
		return false;
	}

#ifndef LM_MPI
	if (mpiMode)
	{
		LM_LOG_ERROR("Invalid 'mpi' argument. The application is not build for MPI.")
	}
#endif

	return true;
}

bool LightmetricaApplication::Initialize( int argc, char** argv )
{
#if LM_MPI
	if (mpiMode)
	{
		if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
		{
			std::cerr << "Failed to initialize MPI" << std::endl;
			return EXIT_FAILURE;
		}

		// TODO : Create own error handler?
		//MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	}
#endif

	useProgressBar = !mpiMode || (mpiMode && rank == 0);

#if LM_STRICT_FP && LM_PLATFORM_WINDOWS
	_set_se_translator(SETransFunc);
	if (!FloatintPointUtils::EnableFPControl())
	{
		return false;
	}
#endif

	return true;
}

bool LightmetricaApplication::Run()
{
	PrintStartMessage();

	// --------------------------------------------------------------------------------

	// # Load plugins
	{
		LM_LOG_INFO("Entering : Loading plugins");
		LM_LOG_INDENTER();
		ComponentFactory::LoadPlugins(".");
	}

	// --------------------------------------------------------------------------------

	// # Load configuration
	std::unique_ptr<Config> config(ComponentFactory::Create<Config>());
	if (!LoadConfiguration(*config))
	{
		return false;
	}

	// --------------------------------------------------------------------------------

	// # Load assets
	std::unique_ptr<Assets> assets(ComponentFactory::Create<Assets>());
	if (!LoadAssets(*config, *assets))
	{
		return false;
	}

	// --------------------------------------------------------------------------------

	// # Create and setup scene
	auto sceneType = config->Root().Child("scene").AttributeValue("type");
	std::unique_ptr<Scene> scene(ComponentFactory::Create<Scene>(sceneType));
	if (scene == nullptr)
	{
		LM_LOG_ERROR("Invalid scene type ''" + sceneType + "'");
		return false;
	}
	if (!LoadAndBuildScene(*config, *assets, *scene))
	{
		return false;
	}

	// --------------------------------------------------------------------------------

	// # Create and configure renderer
	auto rendererType = config->Root().Child("renderer").AttributeValue("type");
	std::unique_ptr<Renderer> renderer(ComponentFactory::Create<Renderer>(rendererType));
	if (renderer == nullptr)
	{
		LM_LOG_ERROR("Invalid renderer type ''" + rendererType + "'");
		return false;
	}
	if (!ConfigureAndDispatchRenderer(*config, *assets, *scene, *renderer))
	{
		return false;
	}

	// --------------------------------------------------------------------------------

	PrintFinishMessage();

	return true;
}

bool LightmetricaApplication::LoadConfiguration( Config& config )
{
	LM_LOG_INFO("Entering : Configuration loading");
	LM_LOG_INDENTER();

	if (interactiveMode)
	{
		LM_LOG_INFO("Interactive mode ...");

		// Get scene configuration from standard input
		std::string content;
		int c;
		while ((c = std::getchar()) != EOF)
		{
			content += static_cast<char>(c);
		}

		// Load configuration from string
		if (!config.LoadFromString(content, basePath))
		{
			return false;
		}
	}
	else
	{
		if (!config.Load(inputFile, basePath))
		{
			return false;
		}
	}

	return true;
}

bool LightmetricaApplication::LoadAssets( const Config& config, Assets& assets )
{
	// # Register component interfaces
	assets.RegisterInterface<Texture>();
	assets.RegisterInterface<BSDF>();
	assets.RegisterInterface<TriangleMesh>();
	assets.RegisterInterface<Film>();
	assets.RegisterInterface<Camera>();
	assets.RegisterInterface<Light>();

	// --------------------------------------------------------------------------------

	// # Load assets
	{
		LM_LOG_INFO("Entering : Asset loading");
		LM_LOG_INDENTER();

		if (useProgressBar)
		{
			progressBar.Begin("LOADING ASSETS");
			auto conn = assets.Connect_ReportProgress(std::bind(&ProgressBar::OnReportProgress, &progressBar, std::placeholders::_1, std::placeholders::_2));
		}

		if (!assets.Load(config.Root().Child("assets")))
		{
			progressBar.Abort();
			return false;
		}

		if (useProgressBar)
		{
			progressBar.End();
		}
	}

	// --------------------------------------------------------------------------------

	return true;
}

bool LightmetricaApplication::LoadAndBuildScene( const Config& config, const Assets& assets, Scene& scene )
{
	// # Load primitives
	{
		std::unique_ptr<Primitives> primitives(ComponentFactory::Create<Primitives>());
		if (primitives == nullptr)
		{
			return false;
		}

		LM_LOG_INFO("Entering : Primitive loading");
		LM_LOG_INDENTER();
		
		if (!primitives->Load(config.Root().Child("scene"), assets))
		{
			return false;
		}

		// Load #primitives to #scene. #primitives are managed by #scene
		scene.Load(primitives.release());
	}

	// --------------------------------------------------------------------------------

	// # Configure scene
	{
		LM_LOG_INFO("Entering : Scene configuration");
		LM_LOG_INDENTER();
		LM_LOG_INFO("Scene type : '" + scene.ComponentImplTypeName() + "'");
		if (!scene.Configure(config.Root().Child("scene")))
		{
			return false;
		}
	}

	// --------------------------------------------------------------------------------

	// # Build scene
	{
		LM_LOG_INFO("Entering : Scene building");
		LM_LOG_INDENTER();

		if (useProgressBar)
		{
			progressBar.Begin("BUILDING SCENE");
			auto conn = scene.Connect_ReportBuildProgress(std::bind(&ProgressBar::OnReportProgress, &progressBar, std::placeholders::_1, std::placeholders::_2));
		}

		if (!scene.Build())
		{
			progressBar.Abort();
			return false;
		}

		if (useProgressBar)
		{
			progressBar.End();
		}
	}

	// --------------------------------------------------------------------------------

	// # Post configuration
	{
		LM_LOG_INFO("Entering : Scene post configuration");
		LM_LOG_INDENTER();
		if (!scene.PostConfigure())
		{
			return false;
		}
	}

	// --------------------------------------------------------------------------------

	return true;
}

bool LightmetricaApplication::ConfigureAndDispatchRenderer( const Config& config, const Assets& assets, const Scene& scene, Renderer& renderer )
{
	// # Configure renderer
	{
		LM_LOG_INFO("Entering : Renderer configuration");
		LM_LOG_INDENTER();
	
		// Configure
		LM_LOG_INFO("Renderer type : '" + renderer.Type() + "'");
		if (!renderer.Configure(config.Root().Child("renderer"), assets))
		{
			return false;
		}

		// Termination mode
		LM_LOG_INFO("Termination mode : " + std::string(terminationTime == 0 ? "Samples" : "Time"));
		renderer.SetTerminationMode(terminationTime == 0 ? TerminationMode::Samples : TerminationMode::Time, terminationTime);
	}

	// --------------------------------------------------------------------------------

	// # Preprocess renderer
	{
		LM_LOG_INFO("Entering : Preprocess");
		LM_LOG_INDENTER();

		if (useProgressBar)
		{
			progressBar.Begin("PREPROCESS");
			auto conn = renderer.Connect_ReportProgress(std::bind(&ProgressBar::OnReportProgress, &progressBar, std::placeholders::_1, std::placeholders::_2));
		}

		if (!renderer.Preprocess(scene))
		{
			progressBar.Abort();
			return false;
		}

		if (useProgressBar)
		{
			progressBar.End();
		}
	}

	// --------------------------------------------------------------------------------

	// # Begin rendering
	{
		LM_LOG_INFO("Entering : Render");
		LM_LOG_INDENTER();

		if (useProgressBar)
		{
			progressBar.Begin("RENDERING");
			auto conn = renderer.Connect_ReportProgress(std::bind(&ProgressBar::OnReportProgress, &progressBar, std::placeholders::_1, std::placeholders::_2));
		}

		if (!renderer.Render(scene))
		{
			progressBar.Abort();
			return false;
		}

		if (useProgressBar)
		{
			progressBar.End();
		}
	}

	// --------------------------------------------------------------------------------

	// # Save rendered image
	{
		if (!mpiMode || (mpiMode && rank == 0))
		{
			LM_LOG_INFO("Entering : Save rendered image");
			LM_LOG_INDENTER();
			auto* film = dynamic_cast<BitmapFilm*>(scene.MainCamera()->GetFilm());
			if (film == nullptr)
			{
				LM_LOG_WARN("Main camera is not associated with bitmap texture, skipping");
			}
			else
			{
				if (!film->Save(outputImagePath))
				{
					return false;
				}
			}
		}
	}

	// --------------------------------------------------------------------------------

	return true;
}

void LightmetricaApplication::StartLogging()
{
	// Configure the logger
	if (mpiMode)
	{
		int rank;
#if LM_MPI
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif
		Logger::SetOutputFileName(boost::str(boost::format("lightmetrica.%02d.log") % rank));
		if (rank == 0)
		{
			Logger::SetOutputMode(Logger::LogOutputMode::Stdout | Logger::LogOutputMode::File);
		}
		else
		{
			Logger::SetOutputMode(Logger::LogOutputMode::File);
		}
	}
	else
	{
		Logger::SetOutputMode(Logger::LogOutputMode::Stdout | Logger::LogOutputMode::File);
	}

	// Start the logger thread
	logResult = std::async(
		std::launch::async,
		[this]()
		{
			// Console info
			int consoleWidth;
			if (mpiMode)
			{
				consoleWidth = 72;
			}
			else
			{
#if LM_PLATFORM_WINDOWS
				HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
				CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
				GetConsoleScreenBufferInfo(consoleHandle, &screenBufferInfo);
				consoleWidth = screenBufferInfo.dwSize.X-1;
#elif LM_PLATFORM_LINUX
				struct winsize w;
				ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
				consoleWidth = w.ws_col;
#endif
			}

			if (useProgressBar)
			{
				progressBar.SetConsoleWidth(consoleWidth);
			}

			std::string spaces(consoleWidth, ' ');

			// Event loop for logger process
			while (!logThreadDone || !Logger::Empty())
			{
				// Process log output
				// Overwrite a line with spaces
				if (!Logger::Empty())
				{
					if (useProgressBar)
					{
						std::cout << spaces << "\r";
						Logger::ProcessOutput();
						progressBar.RequestUpdateProgress();
					}
					else
					{
						Logger::ProcessOutput();
					}
				}

				// Process progress bar
				if (useProgressBar)
				{
					progressBar.ProcessProgressOutput();
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		});
}

void LightmetricaApplication::FinishLogging()
{
	// Wait for the finish of the logger thread
	logThreadDone = true;
	logResult.wait();
}

void LightmetricaApplication::PrintStartMessage()
{
	LM_LOG_INFO("");
	LM_LOG_INFO(appDescription);
	LM_LOG_INFO("");
	LM_LOG_INFO("Copyright (c) 2014 Hisanari Otsu (hi2p.perim@gmail.com)");
	LM_LOG_INFO("The software is distributed under GPLv3.");
	LM_LOG_INFO("For detail see the LICENSE file along with the software.");
	LM_LOG_INFO("");
	LM_LOG_INFO("BUILD DATE   | " + Version::BuildDate());
	LM_LOG_INFO("PLATFORM     | " + Version::Platform() + " " + Version::Archtecture());
	LM_LOG_INFO("FLAGS        | " + appFlags);
	LM_LOG_INFO("CURRENT TIME | " + CurrentTime());
	LM_LOG_INFO("");

#if LM_MPI
	if (mpiMode)
	{
		int numProcs;
		int procNameLen;
		char procName[MPI_MAX_PROCESSOR_NAME];
		MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
		MPI_Get_processor_name(procName, &procNameLen);
		LM_LOG_INFO("MPI mode");
		LM_LOG_INFO("PROCESS NUM  | " + std::to_string(numProcs));
		LM_LOG_INFO("PROCESS RANK | " + std::to_string(rank));
		LM_LOG_INFO("PROCESS NAME | " + std::string(procName));
		LM_LOG_INFO("");
	}
#endif
}

void LightmetricaApplication::PrintFinishMessage()
{
	LM_LOG_INFO("Completed");
}

std::string LightmetricaApplication::CurrentTime()
{
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
#if LM_PLATFORM_WINDOWS
	struct tm timeinfo;
	localtime_s(&timeinfo, &time);
	ss << std::put_time(&timeinfo, "%Y.%m.%d.%H.%M.%S");
#elif LM_PLATFORM_LINUX
	// std::put_time is not implemented
	char timeStr[256];
	std::strftime(timeStr, sizeof(timeStr), "%Y.%m.%d.%H.%M.%S", std::localtime(&time));
	ss << timeStr;
#endif
	return ss.str();
}

#if LM_PLATFORM_WINDOWS
void LightmetricaApplication::SETransFunc(unsigned int code, PEXCEPTION_POINTERS data)
{
	std::string desc;
	switch (code)
	{
		case EXCEPTION_ACCESS_VIOLATION:			{ desc = "EXCEPTION_ACCESS_VIOLATION";			break; }
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:		{ desc = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";		break; }
		case EXCEPTION_BREAKPOINT:					{ desc = "EXCEPTION_BREAKPOINT";				break; }
		case EXCEPTION_DATATYPE_MISALIGNMENT:		{ desc = "EXCEPTION_DATATYPE_MISALIGNMENT";		break; }
		case EXCEPTION_FLT_DENORMAL_OPERAND:		{ desc = "EXCEPTION_FLT_DENORMAL_OPERAND";		break; }
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:			{ desc = "EXCEPTION_FLT_DIVIDE_BY_ZERO";		break; }
		case EXCEPTION_FLT_INEXACT_RESULT:			{ desc = "EXCEPTION_FLT_INEXACT_RESULT";		break; }
		case EXCEPTION_FLT_INVALID_OPERATION:		{ desc = "EXCEPTION_FLT_INVALID_OPERATION";		break; }
		case EXCEPTION_FLT_OVERFLOW:				{ desc = "EXCEPTION_FLT_OVERFLOW";				break; }
		case EXCEPTION_FLT_STACK_CHECK:				{ desc = "EXCEPTION_FLT_STACK_CHECK";			break; }
		case EXCEPTION_FLT_UNDERFLOW:				{ desc = "EXCEPTION_FLT_UNDERFLOW";				break; }
		case EXCEPTION_ILLEGAL_INSTRUCTION:			{ desc = "EXCEPTION_ILLEGAL_INSTRUCTION";		break; }
		case EXCEPTION_IN_PAGE_ERROR:				{ desc = "EXCEPTION_IN_PAGE_ERROR";				break; }
		case EXCEPTION_INT_DIVIDE_BY_ZERO:			{ desc = "EXCEPTION_INT_DIVIDE_BY_ZERO";		break; }
		case EXCEPTION_INT_OVERFLOW:				{ desc = "EXCEPTION_INT_OVERFLOW";				break; }
		case EXCEPTION_INVALID_DISPOSITION:			{ desc = "EXCEPTION_INVALID_DISPOSITION";		break; }
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:	{ desc = "EXCEPTION_NONCONTINUABLE_EXCEPTION";	break; }
		case EXCEPTION_PRIV_INSTRUCTION:			{ desc = "EXCEPTION_PRIV_INSTRUCTION";			break; }
		case EXCEPTION_SINGLE_STEP:					{ desc = "EXCEPTION_SINGLE_STEP";				break; }
		case EXCEPTION_STACK_OVERFLOW:				{ desc = "EXCEPTION_STACK_OVERFLOW";			break; }
	}

	LM_LOG_ERROR("Structured exception is detected");
	LM_LOG_INDENTER();
	LM_LOG_ERROR("Exception code    : " + boost::str(boost::format("0x%08x") % code));
	LM_LOG_ERROR("Exception address : " + boost::str(boost::format("0x%08x") % data->ExceptionRecord->ExceptionAddress));
	if (!desc.empty())
	{
		LM_LOG_ERROR("Description       : " + desc);
	}

#if LM_DEBUG_MODE
	__debugbreak();
#endif

	throw std::runtime_error("Aborting");
}
#endif

int main(int argc, char** argv)
{
	int result = EXIT_SUCCESS;
	LightmetricaApplication app;

	if (app.ParseArguments(argc, argv) && app.Initialize(argc, argv))
	{
		app.StartLogging();

#if LM_MPI && LM_DEBUG_MODE
		int rank;
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		if (rank == 0)
		{
			std::cerr << "Wait for attaching. If you are prepared, press any key.";
			std::cin.get();
		}
		MPI_Barrier(MPI_COMM_WORLD);
#endif

		try
		{
			if (!app.Run())
			{
				result = EXIT_FAILURE;
			}
		}
		catch (const std::exception& e)
		{
			LM_LOG_ERROR(boost::str(boost::format("EXCEPTION | %s") % e.what()));
			result = EXIT_FAILURE;
		}

		app.FinishLogging();
	}

	// --------------------------------------------------------------------------------

#if LM_DEBUG_MODE
#if LM_MPI
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0)
	{
		std::cerr << "Press any key to exit ...";
		std::cin.get();
	}
	MPI_Barrier(MPI_COMM_WORLD);
#else
	std::cerr << "Press any key to exit ...";
	std::cin.get();
#endif
#endif

#if LM_MPI
	MPI_Finalize();
#endif

	return result;
}
