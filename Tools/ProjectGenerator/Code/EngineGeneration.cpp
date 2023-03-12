#include "EngineGeneration.h"
#include <filesystem>
#include <fstream>
#include <map>
#include "Log.h"
#include "Util.h"
#include "SLNGenerator.h"
#include <UI/UIButton.h>
#include <Rendering/Texture.h>
#include <UI/UIBackground.h>
#include <Application.h>

std::map<std::string, std::string> LibraryPaths =
{
	std::pair("SDL2 include path", ""),
	std::pair("SDL2 library path", ""),
	std::pair("glew include path", ""),
	std::pair("glew library path", ""),
	std::pair("assimp include path", ""),
	std::pair("assimp library path", ""),
	std::pair("OpenAL include path", ""),
	std::pair("OpenAL library path", ""),
	std::pair("glm include path", ""),
};

std::map<std::string, std::string> LoadedPaths =
{
	std::pair("sdl_include", "SDL2 include path"),
	std::pair("sdl_lib", "SDL2 library path"),
	std::pair("glew_include", "glew include path"),
	std::pair("glew_lib", "glew library path"),
	std::pair("assimp_include", "assimp include path"),
	std::pair("assimp_lib", "assimp library path"),
	std::pair("al_include", "OpenAL include path"),
	std::pair("al_lib", "OpenAL library path"),
	std::pair("glm_include", "glm include path"),
};

std::map<std::string, std::string> LibraryFiles =
{
	std::pair("SDL2 include path", "SDL.h"),
	std::pair("SDL2 library path", "SDL2.lib"),
	std::pair("glew include path", "GL/glew.h"),
	std::pair("glew library path", "glew32s.lib"),
	std::pair("assimp include path", "assimp/aabb.h"),
	std::pair("assimp library path", "assimp.lib"),
	std::pair("OpenAL include path", "AL/al.h"),
	std::pair("OpenAL library path", "OpenAL32.lib"),
	std::pair("glm include path", "glm/glm.hpp"),
};

void GenerateFiles()
{
	//std::filesystem::current_path("../");
	std::string compiler = "msvc";
	std::ofstream Out = std::ofstream("../cppinfo.txt");
	Out << "build_system=" << compiler << std::endl;
	Out.close();
	std::map<std::string, std::string> Values;
	Values.insert(std::pair("sdl_include", LibraryPaths["SDL2 include path"]));
	Values.insert(std::pair("sdl_lib", LibraryPaths["SDL2 library path"]));
	Values.insert(std::pair("glm_include", LibraryPaths["glm include path"]));
	Values.insert(std::pair("glew_include", LibraryPaths["glew include path"]));
	Values.insert(std::pair("glew_lib", LibraryPaths["glew library path"]));
	Values.insert(std::pair("assimp_include", LibraryPaths["assimp include path"]));
	Values.insert(std::pair("assimp_lib", LibraryPaths["assimp library path"]));
	Values.insert(std::pair("al_include", LibraryPaths["OpenAL include path"]));
	Values.insert(std::pair("al_lib", LibraryPaths["OpenAL library path"]));
	Out = std::ofstream("../paths.txt");

	for (auto v : Values)
	{
		Out << v.first << "=" << v.second << std::endl;
	}
	Out.close();
	if (compiler == "msvc")
	{
		try
		{
			auto f = Util::GetAllFilesInFolder("../EngineSource", true, true);
			for (auto i : f)
			{
				Log::Print("Found engine source code: " + i);
			}
			Log::Print("Generating .sln and .vcxproj files...");
			sln::GenerateSolution("../", "Engine", Values, f, "../EngineSource/");
			Util::Notify("Created project files. \n\
Please compile the Editor, Debug and Release libraries, then run this program again to create a new project.");
			Application::Quit = true;
			return;
		}
		catch (Exception& e)
		{
			Log::Print("Error while trying to create a solution: " + e.What(), Log::E_ERROR);
			return;
		}
		catch (std::exception& e)
		{
			Log::Print("Error while trying to create a solution: " + std::string(e.what()), Log::E_ERROR);
			return;
		}
	}
	//std::filesystem::current_path("Launcher");
}

namespace Background
{
	extern std::thread* BackgroundThread;
	extern std::atomic<float> BackgroundProgress;
	extern std::string BackgroundTask;
}

void AskForInstallation(std::string Filepath)
{
	Background::BackgroundTask = "Setting filepath";

	LibraryPaths[Filepath] = Util::ShowSelectFolderDialog();
	Background::BackgroundProgress = 1;
}

namespace Installation
{
	UIBox* TextBackground = nullptr;
	unsigned int LoadFileTexture;
	TextRenderer* Text;
	std::vector<UIButton*> FileButtons;
}

void Installation::ManageFirstInstall(TextRenderer* t)
{
	Log::Print("A new installation has been detected");
	Text = t;
	TextBackground = new UIBox(false, Vector2f(-0.9, -0.4));
	TextBackground->Align = UIBox::E_REVERSE;
	TextBackground->SetMaxSize(1);
	TextBackground->SetMinSize(1);
	LoadFileTexture = Texture::LoadTexture("Textures/Folder.png");

	(new UIButton(true, Vector2f32(-0.9, -0.9), Vector3f32(0, 0.85, 0), []() {GenerateFiles(); }))
		->SetBorder(UIBox::E_ROUNDED, 0.75)
		->AddChild((new UIText(0.4, 0, "Generate", Text))
			->SetPadding(0.03));

	if (std::filesystem::exists("../paths.txt"))
	{
		Log::Print("Detected previous installation. Copying old file paths to libraries from paths.txt", Log::E_WARNING);
		std::ifstream PathsFile = std::ifstream("../paths.txt");
		while (!PathsFile.eof())
		{
			char ReadBuffer[256];
			PathsFile.getline(ReadBuffer, 256 * sizeof(char));
			std::string ReadString = ReadBuffer;
			size_t Seperator = ReadString.find_first_of("=");
			std::string a = ReadString.substr(0, Seperator), b = ReadString.substr(Seperator + 1);
			if (LoadedPaths.contains(a))
			{
				LibraryPaths[LoadedPaths[a]] = b;
			}
		}
	}
	UpdateLibrayPaths();
}

void Installation::UpdateLibrayPaths()
{
	TextBackground->DeleteChildren();
	TextBackground->AddChild((new UIBackground(true, 0, 0.4, Vector2f(1, 0.01)))->SetPadding(0));
	FileButtons.clear();
	for (auto& i : LibraryPaths)
	{
		UIBox* b = new UIBox(true, 0);
		std::string s = i.first;
		s.resize(40, ' ');
		b->AddChild((new UIBox(false, 0))
			->AddChild((new UIText(0.275, std::filesystem::exists(i.second + "/" + LibraryFiles[i.first]) ? Vector3f32(0, 0.5, 0) : Vector3f32(0.5, 0, 0),
				i.second.empty() ? "[No path selected]" : i.second, Text))->SetPadding(0.01, 0.01, 0, 0))
			->AddChild((new UIText(0.4, 0, s, Text))->SetPadding(0)));

		UIButton* Button = (new UIButton(true, 0, Vector3f32(1, 0.8, 0), []() {
			if (Background::BackgroundThread)
			{
				return;
			}
			for (size_t i = 0; i < FileButtons.size(); i++)
			{
				if (!FileButtons[i]->GetIsHovered())
				{
					continue;
				}

				size_t it = 0;
				for (auto& j : LibraryPaths)
				{
					if (it++ == i)
					{
						Background::BackgroundThread = new std::thread(AskForInstallation, j.first);
						return;
					}
				}
			}
		}));
		FileButtons.push_back(Button);
		b->AddChild(Button
			->SetUseTexture(true, LoadFileTexture)
			->SetMinSize(0.08)
			->SetPadding(0)
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE));
		TextBackground->AddChild(b);
		TextBackground->AddChild((new UIBackground(true, 0, 0.4, Vector2f(1, 0.01)))->SetPadding(0));
	}
}
