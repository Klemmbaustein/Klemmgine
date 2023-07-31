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
#include <filesystem>

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
	for (auto& i : LibraryPaths)
	{
		if (i.second.empty())
		{
			Util::Notify("Not all paths have been specified.");
			return;
		}
	}

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

	for (auto& v : Values)
	{
		Out << v.first << "=" << v.second << std::endl;
	}
	Out.close();
	if (compiler == "msvc")
	{
		try
		{
			auto f = Util::GetAllFilesInFolder("../EngineSource", true, true);
			for (auto& i : f)
			{
				Log::Print("Found engine source code: " + i);
			}


			Log::Print("Generating .sln and .vcxproj files...");
			sln::GenerateSolution("../", "Engine", Values, f, std::filesystem::exists("../CSharpCore"), "../EngineSource/");

			if (std::filesystem::exists("../CSharpCore"))
			{
				system("cd ../CSharpCore && dotnet build");
			}

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

void LoadLibraryPackage(std::string Package)
{
	std::map<std::string, std::string> Folders =
	{
		std::pair("Assimp", "assimp"),
		std::pair("GLEW", "glew"),
		std::pair("glm", "glm"),
		std::pair("OpenAL", "OpenAL"),
		std::pair("SDL2", "SDL2")
	};
	if (Package.empty()) return;
	for (auto& i : Folders)
	{
		if (!std::filesystem::exists(Package + "/" + i.first))
		{
			Util::Notify(Package + " does not have all required libraries.\nMissing: " + i.first);
			return;
		}
		if (!std::filesystem::exists(Package + "/" + i.first + "/include"))
		{
			Util::Notify(Package + " does not have all required files.\nMissing: " + i.first + " include files");
			return;
		}
		if (!std::filesystem::exists(Package + "/" + i.first + "/lib") && i.first != "glm")
		{
			Util::Notify(Package + " does not have all required files.\nMissing: " + i.first + " library files");
			return;
		}
	}
	
	for (auto& i : Folders)
	{
		LibraryPaths[i.second + " include path"] = std::filesystem::absolute(Package + "/" + i.first + "/include").string();
		if (i.first != "glm")
		{
			LibraryPaths[i.second + " library path"] = std::filesystem::absolute(Package + "/" + i.first + "/lib").string();
		}
	}
	Installation::UpdateLibrayPaths();
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
	UIBox* PackageBackground = nullptr;
}

void Installation::ManageFirstInstall(TextRenderer* t)
{
	Log::Print("A new installation has been detected");
	Text = t;
	auto HorizontalBox = new UIBox(true, Vector2f(-0.95, -0.4));

	TextBackground = new UIBox(false, Vector2f(-0.95, -0.4));
	TextBackground->Align = UIBox::E_REVERSE;
	TextBackground->SetMaxSize(1);
	TextBackground->SetMinSize(1);
	TextBackground->SetPadding(0);
	HorizontalBox->AddChild(TextBackground);
	HorizontalBox->AddChild((new UIBackground(true, 0, 0.4f, Vector2f(0.005, 1)))
		->SetPadding(0, 0, 0.05, 0.05));

	PackageBackground = new UIBox(false, 0);
	HorizontalBox->AddChild(PackageBackground
		->AddChild((new UIText(0.5, 0, "or provide path to library package", Text))
			->SetPadding(0))
		->AddChild((new UIButton(true, 0, Vector3f32(0, 0.8f, 0), []() { LoadLibraryPackage(Util::ShowSelectFolderDialog()); }))
			->SetBorder(UIBox::E_ROUNDED, 0.75)
			->AddChild((new UIText(0.4f, 0, "Open library package", Text))->SetPadding(0.025))));
	LoadFileTexture = Texture::LoadTexture("Textures/Folder.png");
	PackageBackground->Align = UIBox::E_REVERSE;
	PackageBackground->SetMinSize(1);
	PackageBackground->SetMaxSize(1);
	(new UIButton(true, Vector2f32(-0.9f, -0.9f), Vector3f32(0, 0.85f, 0), []() {GenerateFiles(); }))
		->SetBorder(UIBox::E_ROUNDED, 0.75)
		->AddChild((new UIText(0.4, 0, "Generate", Text))
			->SetPadding(0.03));

	(new UIText(0.8, 0, "Installation", Text))
		->SetPosition(Vector2f(-0.9, 0.8));

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

#define MAX_PATH_LENGTH 60

void Installation::UpdateLibrayPaths()
{
	TextBackground->DeleteChildren();
	TextBackground->AddChild(new UIText(0.5, 0, "Enter paths to required libraries here", Text));
	TextBackground->AddChild((new UIBackground(true, 0, 0.4, Vector2f(1, 0.01)))->SetPadding(0));
	FileButtons.clear();
	for (std::pair i : LibraryPaths)
	{
		UIBox* b = new UIBox(true, 0);
		std::string s = i.first;
		s.resize(44, ' ');
		std::string PathText = i.second;
		if (PathText.size() > MAX_PATH_LENGTH - 3)
		{
			PathText = PathText.substr(0, MAX_PATH_LENGTH - 3) + "...";
		}


		b->AddChild((new UIBox(false, 0))
			->AddChild((new UIText(0.275, std::filesystem::exists(i.second + "/" + LibraryFiles[i.first]) ? Vector3f32(0, 0.5, 0) : Vector3f32(0.5, 0, 0),
				i.second.empty() ? "[No path selected]" : PathText, Text))->SetPadding(0))
			->AddChild((new UIText(0.4, 0, s, Text))->SetPadding(-0.01, -0.01, 0, 0)));

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
		TextBackground->AddChild((new UIBackground(true, 0, 0.4f, Vector2f(1, 0.01)))->SetPadding(0));
	}
}
