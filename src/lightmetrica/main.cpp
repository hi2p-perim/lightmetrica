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

#include <lightmetrica/config.h>
#include <lightmetrica/rendererdispatcher.h>
#include <lightmetrica/logger.h>
#include <lightmetrica/defaultassets.h>
#include <lightmetrica/version.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/scenefactory.h>
#include <lightmetrica/renderer.h>
#include <lightmetrica/rendererfactory.h>
#include <lightmetrica/camerafactory.h>
#include <lightmetrica/filmfactory.h>
#include <lightmetrica/lightfactory.h>
#include <lightmetrica/bsdffactory.h>
#include <lightmetrica/texturefactory.h>
#include <lightmetrica/trianglemeshfactory.h>
#include <lightmetrica/math.h>
#include <lightmetrica/camera.h>
#include <lightmetrica/film.h>
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
#ifdef LM_PLATFORM_WINDOWS
#include <windows.h>
#endif

using namespace lightmetrica;
namespace po = boost::program_options;

template <typename BoolType>
class scoped_enable
{
public:

	scoped_enable(BoolType& f) : flag(f) { flag = true; }
	~scoped_enable() { flag = false; }

private:

	BoolType& flag;

};

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

	void SetAppInfo();
	void PrintHelpMessage(const po::options_description& opt);
	void PrintStartMessage();
	void PrintFinishMessage();
	std::string CurrentTime();
	void OnReportProgress(double progress, bool done);
	void ResetProgress(const std::string& taskName);

private:

	// Application info
	std::string appName;
	std::string appDescription;
	std::string appFlags;

	// Command line parameters
	std::string inputFile;

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

NanonApplication::NanonApplication()
	: logThreadDone(false)
{
	SetAppInfo();
}

void NanonApplication::SetAppInfo()
{
	appName = "Lightmetrica";
	appDescription = boost::str(boost::format("%s Version %s (%s)") % appName % Version::Formatted() % Version::Codename());
	
	// Enumerate flags
	std::vector<std::string> flags;
#ifdef LM_SINGLE_PRECISION
	flags.push_back("single_precision");
#endif
#ifdef LM_DOUBLE_PRECISION
	flags.push_back("double_precision");
#endif
#ifdef LM_MULTI_PRECISION
	flags.push_back("multi_precision");
#endif
#ifdef LM_USE_SSE
	flags.push_back("sse");
#endif
#ifdef LM_USE_SSE2
	flags.push_back("sse2");
#endif
#ifdef LM_USE_SSE3
	flags.push_back("sse3");
#endif
#ifdef LM_USE_SSSE3
	flags.push_back("ssse3");
#endif
#ifdef LM_USE_SSE4_1
	flags.push_back("sse4.1");
#endif
#ifdef LM_USE_SSE4_2
	flags.push_back("sse4.2");
#endif
#ifdef LM_USE_SSE4A
	flags.push_back("sse4a");
#endif
#ifdef LM_USE_AVX
	flags.push_back("avx");
#endif

	appFlags = "";
	for (size_t i = 0; i < flags.size(); i++)
	{
		appFlags += flags[i];
		if (i < flags.size() - 1)
		{
			appFlags += " ";
		}
	}
}

void NanonApplication::PrintHelpMessage( const po::options_description& opt )
{
	std::cout << appDescription << std::endl;
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

	// All positional position are translated into --input-file
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

	// --------------------------------------------------------------------------------

	// Load input file
	NanonConfig config;
	LM_LOG_INFO("Entering : Configuration loading");
	{
		LM_LOG_INDENTER();
		if (!config.Load(inputFile))
		{
			return false;
		}
		LM_LOG_INFO("Completed");
	}
	LM_LOG_INFO("Leaving : Configuration loading");

	// --------------------------------------------------------------------------------

	// Load assets
	DefaultAssets assets;

	// Register default asset factories
	assets.RegisterAssetFactory(AssetFactoryEntry("textures", "texture", 0, new TextureFactory));
	assets.RegisterAssetFactory(AssetFactoryEntry("bsdfs", "bsdf", 1, new BSDFFactory));
	assets.RegisterAssetFactory(AssetFactoryEntry("triangle_meshes", "triangle_mesh", 1, new TriangleMeshFactory));
	assets.RegisterAssetFactory(AssetFactoryEntry("films", "film", 1, new FilmFactory));
	assets.RegisterAssetFactory(AssetFactoryEntry("cameras", "camera", 1, new CameraFactory));
	assets.RegisterAssetFactory(AssetFactoryEntry("lights", "light", 1, new LightFactory));

	LM_LOG_INFO("Entering : Asset loading")
	{
		ResetProgress("LOADING ASSETS");
		scoped_enable<std::atomic<bool>> _(enableProgressBar);
		auto conn = assets.Connect_ReportProgress(std::bind(&NanonApplication::OnReportProgress, this, std::placeholders::_1, std::placeholders::_2));
		
		LM_LOG_INDENTER();
		if (!assets.Load(config))
		{
			return false;
		}
		LM_LOG_INFO("Completed");

		{
			std::unique_lock<std::mutex> lock(progressMutex);
			progressDoneCond.wait(lock, [this](){ return progressPrintDone; });
		}
	}
	LM_LOG_INFO("Leaving : Asset loading");

	// --------------------------------------------------------------------------------

	// Create scene
	SceneFactory sceneFactory;
	std::unique_ptr<Scene> scene(sceneFactory.Create(config.SceneType()));
	if (scene == nullptr)
	{
		return false;
	}

	// Load scene
	LM_LOG_INFO("Entering : Scene loading");
	{
		LM_LOG_INDENTER();
		if (!scene->Load(config, assets))
		{
			return false;
		}
		LM_LOG_INFO("Completed");
	}
	LM_LOG_INFO("Leaving : Scene loading");

	// Configure scene
	LM_LOG_INFO("Entering : Scene configuration");
	{
		LM_LOG_INDENTER();
		LM_LOG_INFO("Scene type : '" + scene->Type() + "'");
		if (!scene->Configure(config))
		{
			return false;
		}
		LM_LOG_INFO("Completed");
	}
	LM_LOG_INFO("Leaving : Scene configuration");

	// Build scene
	LM_LOG_INFO("Entering : Scene building");
	{
		ResetProgress("BUILDING SCENE");
		scoped_enable<std::atomic<bool>> _(enableProgressBar);
		auto conn = scene->Connect_ReportBuildProgress(std::bind(&NanonApplication::OnReportProgress, this, std::placeholders::_1, std::placeholders::_2));
		
		LM_LOG_INDENTER();
		if (!scene->Build())
		{
			return false;
		}
		LM_LOG_INFO("Completed");

		{
			std::unique_lock<std::mutex> lock(progressMutex);
			progressDoneCond.wait(lock, [this](){ return progressPrintDone; });
		}
	}
	LM_LOG_INFO("Leaving : Scene building");

	// --------------------------------------------------------------------------------

	// Create renderer
	RendererFactory rendererFactory;
	std::unique_ptr<Renderer> renderer(rendererFactory.Create(config.RendererType()));
	if (renderer == nullptr)
	{
		return false;
	}

	// Configure renderer
	LM_LOG_INFO("Entering : Renderer configuration");
	{
		LM_LOG_INDENTER();
		LM_LOG_INFO("Renderer type : '" + renderer->Type() + "'");
		if (!renderer->Configure(config, assets))
		{
			return false;
		}
		LM_LOG_INFO("Completed");
	}
	LM_LOG_INFO("Leaving : Renderer configuration");

	// Begin rendering
	LM_LOG_INFO("Entering : Render");
	{
		ResetProgress("RENDERING");
		scoped_enable<std::atomic<bool>> _(enableProgressBar);
		auto conn = renderer->Connect_ReportProgress(std::bind(&NanonApplication::OnReportProgress, this, std::placeholders::_1, std::placeholders::_2));

		LM_LOG_INDENTER();
		if (!renderer->Render(*scene))
		{
			return false;
		}
		LM_LOG_INFO("Completed");

		{
			std::unique_lock<std::mutex> lock(progressMutex);
			progressDoneCond.wait(lock, [this](){ return progressPrintDone; });
		}
	}
	LM_LOG_INFO("Leaving : Render");

	// --------------------------------------------------------------------------------

	// Save rendered image
	LM_LOG_INFO("Entering : Save rendered image");
	{
		LM_LOG_INDENTER();
		auto* film = scene->MainCamera()->GetFilm();
		if (!film->Save())
		{
			return false;
		}
		LM_LOG_INFO("Completed");
	}
	LM_LOG_INFO("Leaving : Save rendered image");

	// --------------------------------------------------------------------------------

	// Finish message
	PrintFinishMessage();

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
			// Console info
			int consoleWidth;
#ifdef LM_PLATFORM_WINDOWS
			HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
			GetConsoleScreenBufferInfo(consoleHandle, &screenBufferInfo);
			consoleWidth = screenBufferInfo.dwSize.X-1;
#else
			consoleWidth = 70;
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
#ifdef LM_PLATFORM_WINDOWS
					SetConsoleTextAttribute(consoleHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
					std::cout << bar;
#ifdef LM_PLATFORM_WINDOWS
					SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
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

void NanonApplication::FinishLogging()
{
	// Wait for the finish of the logger thread
	logThreadDone = true;
	logResult.wait();
}

void NanonApplication::PrintStartMessage()
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

void NanonApplication::PrintFinishMessage()
{
	LM_LOG_INFO("Completed");
}

std::string NanonApplication::CurrentTime()
{
	std::stringstream ss;
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
#ifdef LM_PLATFORM_WINDOWS
	struct tm timeinfo;
	localtime_s(&timeinfo, &time);
	ss << std::put_time(&timeinfo, "%Y.%m.%d.%H.%M.%S");
#else
	// std::put_time is not implemented
	char timeStr[256];
	std::strftime(timeStr, sizeof(timeStr), "%Y.%m.%d.%H.%M.%S", std::localtime(&time));
	ss << timeStr;
#endif
	return ss.str();
}

void NanonApplication::OnReportProgress( double progress, bool done )
{
	if (!progressDone)
	{
		std::unique_lock<std::mutex> lock(progressMutex);
		this->progress = progress;
		requiresProgressUpdate = true;
		progressDone = done;
	}
}

void NanonApplication::ResetProgress(const std::string& taskName)
{
	std::unique_lock<std::mutex> lock(progressMutex);
	progress = 0;
	progressTaskName = taskName;
	progressDone = false;
	requiresProgressUpdate = true;
	progressPrintDone = false;
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
			LM_LOG_ERROR(boost::str(boost::format("EXCEPTION | %s") % e.what()));
			result = EXIT_FAILURE;
		}

		app.FinishLogging();
	}

#ifdef LM_DEBUG_MODE
	std::cerr << "Press any key to exit ...";
	std::cin.get();
#endif

	return result;
}
