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

#include <lightmetrica/defaultconfig.h>
#include <lightmetrica/confignode.h>
#include <lightmetrica/rendererdispatcher.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/defaultassets.h>
#include <lightmetrica/version.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/scenefactory.h>
#include <lightmetrica/renderer.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/bitmapfilm.h>
#include <lightmetrica/light.h>
#include <lightmetrica/bsdf.h>
#include <lightmetrica/texture.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <exception>
#include <thread>
#include <condition_variable>
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

using namespace lightmetrica;
namespace po = boost::program_options;

class LightmetricaApplication
{
public:

	LightmetricaApplication();

public:

	bool ParseArguments(int argc, char** argv);
	bool Run();
	void StartLogging();
	void FinishLogging();

private:

	bool LoadConfiguration(Config& config);
	bool LoadAssets(const Config& config, DefaultAssets& assets);
	bool LoadAndBuildScene(const Config& config, const Assets& assets, Scene& scene);
	bool ConfigureAndDispatchRenderer(const Config& config, const Assets& assets, const Scene& scene, Renderer& renderer);

private:

	void SetAppInfo();
	void PrintHelpMessage(const po::options_description& opt);
	void PrintStartMessage();
	void PrintFinishMessage();
	std::string CurrentTime();

private:

	void BeginProgress(const std::string& taskName);
	void EndProgress();
	void OnReportProgress(double progress, bool done);

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

	// Logging thread related variables
	std::atomic<bool> logThreadDone;
	std::future<void> logResult;

	// Progress bar
	std::atomic<bool> enableProgressBar;
	std::atomic<bool> requiresProgressUpdate;
	bool progressPrintDone;
	bool progressDone;
	double progress;
	std::mutex progressMutex;
	std::string progressTaskName;
	std::condition_variable progressDoneCond;

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
		("base-path,b", po::value<std::string>(&basePath)->default_value(""), "Base path for asset loading");

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

	return true;
}

bool LightmetricaApplication::Run()
{
	PrintStartMessage();

	// Load configuration
	DefaultConfig config;
	if (!LoadConfiguration(config))
	{
		return false;
	}

	// Load assets
	DefaultAssets assets;
	if (!LoadAssets(config, assets))
	{
		return false;
	}

	// Create and setup scene
	SceneFactory sceneFactory;
	std::unique_ptr<Scene> scene(sceneFactory.Create(config.Root().Child("scene").AttributeValue("type")));
	if (scene == nullptr)
	{
		return false;
	}
	if (!LoadAndBuildScene(config, assets, *scene))
	{
		return false;
	}

	// Create and configure renderer
	auto rendererType = config.Root().Child("renderer").AttributeValue("type");
	std::unique_ptr<Renderer> renderer(ComponentFactory::Create<Renderer>(rendererType));
	if (renderer == nullptr)
	{
		LM_LOG_ERROR("Invalid renderer type ''" + rendererType + "'");
		return false;
	}
	if (!ConfigureAndDispatchRenderer(config, assets, *scene, *renderer))
	{
		return false;
	}

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
		char c;
		while ((c = std::getchar()) != EOF)
		{
			content += c;
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

bool LightmetricaApplication::LoadAssets( const Config& config, DefaultAssets& assets )
{
	// Register component interfaces
	assets.RegisterInterface<Texture>();
	assets.RegisterInterface<BSDF>();
	assets.RegisterInterface<TriangleMesh>();
	assets.RegisterInterface<Film>();
	assets.RegisterInterface<Camera>();
	assets.RegisterInterface<Light>();

	// Load assets
	{
		LM_LOG_INFO("Entering : Asset loading");
		LM_LOG_INDENTER();

		BeginProgress("LOADING ASSETS");
		auto conn = assets.Connect_ReportProgress(std::bind(&LightmetricaApplication::OnReportProgress, this, std::placeholders::_1, std::placeholders::_2));

		if (!assets.Load(config.Root().Child("assets")))
		{
			EndProgress();
			return false;
		}

		EndProgress();
	}

	return true;
}

bool LightmetricaApplication::LoadAndBuildScene( const Config& config, const Assets& assets, Scene& scene )
{
	// Load scene
	{
		LM_LOG_INFO("Entering : Scene loading");
		LM_LOG_INDENTER();
		if (!scene.Load(config.Root().Child("scene"), assets))
		{
			return false;
		}
	}

	// Configure scene
	{
		LM_LOG_INFO("Entering : Scene configuration");
		LM_LOG_INDENTER();
		LM_LOG_INFO("Scene type : '" + scene.Type() + "'");
		if (!scene.Configure(config.Root().Child("scene")))
		{
			return false;
		}
	}

	// Build scene
	{
		LM_LOG_INFO("Entering : Scene building");
		LM_LOG_INDENTER();

		BeginProgress("BUILDING SCENE");
		auto conn = scene.Connect_ReportBuildProgress(std::bind(&LightmetricaApplication::OnReportProgress, this, std::placeholders::_1, std::placeholders::_2));

		if (!scene.Build())
		{
			EndProgress();
			return false;
		}

		EndProgress();
	}

	return true;
}

bool LightmetricaApplication::ConfigureAndDispatchRenderer( const Config& config, const Assets& assets, const Scene& scene, Renderer& renderer )
{
	// Configure renderer
	{
		LM_LOG_INFO("Entering : Renderer configuration");
		LM_LOG_INDENTER();
		LM_LOG_INFO("Renderer type : '" + renderer.Type() + "'");
		if (!renderer.Configure(config.Root().Child("renderer"), assets))
		{
			return false;
		}
	}

	// Begin rendering
	{
		LM_LOG_INFO("Entering : Render");
		LM_LOG_INDENTER();

		BeginProgress("RENDERING");
		auto conn = renderer.Connect_ReportProgress(std::bind(&LightmetricaApplication::OnReportProgress, this, std::placeholders::_1, std::placeholders::_2));

		if (!renderer.Render(scene))
		{
			EndProgress();
			return false;
		}

		EndProgress();
	}

	// Save rendered image
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

	return true;
}

void LightmetricaApplication::StartLogging()
{
	// Configure the logger
	Logger::SetOutputMode(Logger::LogOutputMode::Stdout | Logger::LogOutputMode::File);

	// Start the logger thread
	logResult = std::async(
		std::launch::async,
		[this]()
		{
			// Console info
			int consoleWidth;
#if LM_PLATFORM_WINDOWS
			HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
			GetConsoleScreenBufferInfo(consoleHandle, &screenBufferInfo);
			consoleWidth = screenBufferInfo.dwSize.X-1;
#elif LM_PLATFORM_LINUX
			struct winsize w;
			ioctl(0, TIOCGWINSZ, &w);
			consoleWidth = w.ws_col;
#endif

			std::string spaces(consoleWidth, ' ');

			// Event loop for logger process
			while (!logThreadDone || !Logger::Empty())
			{
				// Process log output
				// Overwrite a line with spaces
				if (!Logger::Empty())
				{
					std::cout << spaces << "\r";
					Logger::ProcessOutput();
					requiresProgressUpdate = true;
				}
				
				// Process progress bar
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
					SetConsoleTextAttribute(consoleHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#elif LM_PLATFORM_LINUX
					std::cout << "\033[32m";
#endif
					std::cout << bar;
#if LM_PLATFORM_WINDOWS
					SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
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
	LM_LOG_INFO("The software is distributed under the MIT license.");
	LM_LOG_INFO("For detail see the LICENSE file along with the software.");
	LM_LOG_INFO("");
	LM_LOG_INFO("BUILD DATE   | " + Version::BuildDate());
	LM_LOG_INFO("PLATFORM     | " + Version::Platform() + " " + Version::Archtecture());
	LM_LOG_INFO("FLAGS        | " + appFlags);
	LM_LOG_INFO("CURRENT TIME | " + CurrentTime());
	LM_LOG_INFO("");
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

void LightmetricaApplication::OnReportProgress( double progress, bool done )
{
	if (!progressDone)
	{
		std::unique_lock<std::mutex> lock(progressMutex);
		this->progress = progress;
		requiresProgressUpdate = true;
		progressDone = done;
	}
}

void LightmetricaApplication::BeginProgress(const std::string& taskName)
{
	std::unique_lock<std::mutex> lock(progressMutex);
	progress = 0;
	progressTaskName = taskName;
	progressDone = false;
	requiresProgressUpdate = true;
	progressPrintDone = false;
	enableProgressBar = true;
}

void LightmetricaApplication::EndProgress()
{
	std::unique_lock<std::mutex> lock(progressMutex);
	progressDoneCond.wait(lock, [this](){ return progressPrintDone; });
	enableProgressBar = false;
}

int main(int argc, char** argv)
{
	int result = EXIT_SUCCESS;
	LightmetricaApplication app;

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
			LM_LOG_ERROR(boost::str(boost::format("EXCEPTION | %s") % e.what()));
			result = EXIT_FAILURE;
		}

		app.FinishLogging();
	}

#if LM_DEBUG_MODE
	std::cerr << "Press any key to exit ...";
	std::cin.get();
#endif

	return result;
}
