#include "MSVC/VSProj.h"
#include "MSVC/SLN.h"
#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>
#include "Util.h"

#if _WIN32
std::vector<std::string> DependencyDlls =
{
	"Dependencies/assimp/bin/Release/assimp-vc143-mt.dll",
	"Dependencies/openal-soft/Release/OpenAL32.dll",
	"Dependencies/SDL/VisualC/SDL/x64/Release/SDL2.dll",
	"Dependencies/SDL_net/Build/Release/SDL2_net.dll",
	"CSharp/lib/nethost.dll"
};
#else
std::vector<std::string> DependencyDlls =
{
	"CSharp/lib/libnethost.so"
};
#endif

int main(int argc, char** argv)
{
	std::string ExecPath = argv[0];
	if (ExecPath.find_last_of("/\\") != std::string::npos)
	{
		ExecPath = ExecPath.substr(0, ExecPath.find_last_of("\\/"));
		std::filesystem::current_path(ExecPath);
	}
	std::cout << "Klemmgine Project Generator v1.2";
#if ENGINE_NO_SOURCE
	std::cout << " - without engine source";
#endif
	std::cout << std::endl;

	std::map<std::string, std::string> LaunchArgs =
	{
		std::pair("winSdk", "10.0"),
		std::pair("netVersion", "net8.0"),
		std::pair("toolset", "v143"),
		std::pair("projectName", ""),
		std::pair("includeEngine", "true"),
		std::pair("includeCsharp", "true"),
		std::pair("upgrade", "false"),
		std::pair("onlyBuildFiles", "false"),
		std::pair("ciBuild", "false"),
		std::pair("projectPath", ""),
#if _WIN32
		std::pair("buildSystem", "msbuild"),
#else
		std::pair("buildSystem", "cmake"),
#endif
	};

	std::string CurrentArg = "";
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (!CurrentArg.empty())
			{
				auto Arg = LaunchArgs.find(CurrentArg);
				if (Arg != LaunchArgs.end())
				{
					Arg->second = "true";
				}
				else
				{
					std::cout << "Unknown argument: " << CurrentArg << std::endl;
				}
			}
			CurrentArg = std::string(argv[i] + 1);
		}
		else if (CurrentArg.size())
		{
			auto Arg = LaunchArgs.find(CurrentArg);
			if (Arg != LaunchArgs.end())
			{
				Arg->second = argv[i];
			}
			else
			{
				std::cout << "Unknown argument: " << CurrentArg << std::endl;
			}
			CurrentArg.clear();
		}
	}
	if (!CurrentArg.empty())
	{
		auto Arg = LaunchArgs.find(CurrentArg);
		if (Arg != LaunchArgs.end())
		{
			Arg->second = "true";
		}
		else
		{
			std::cout << "Unknown argument: " << CurrentArg << std::endl;
		}
	}

	std::string ProjectName = LaunchArgs["projectName"];

	if (ProjectName.empty())
	{
		std::cout << "Expected project name. Make sure you passed a project name to the tool like this:\n" << argv[0] << " -projectName MyProject" << std::endl;
		return 1;
	}

	std::string ProjectPath = LaunchArgs["projectPath"];

	if (ProjectPath.empty())
	{
		ProjectPath = "Games/" + ProjectName;
	}

	if (LaunchArgs["onlyBuildFiles"] == "false")
	{
		if (std::filesystem::exists(ProjectPath) && LaunchArgs["upgrade"] == "false")
		{
			std::cout << "Warning: " << ProjectPath << " already exists. Replacing..." << std::endl;
			std::filesystem::remove_all(ProjectPath);
		}
		else
		{
			if (std::filesystem::exists(ProjectPath + "/.vs"))
			{
				std::filesystem::remove_all(ProjectPath + "/.vs");
				std::cout << "Upgrading project files of " << ProjectName << std::endl;
			}
		}

		std::cout << "Copying project files" << std::endl;
		std::filesystem::create_directories(ProjectPath + "/Code/Objects");
		std::filesystem::copy("Tools/ProjectGenerator/ProjectFiles", ProjectPath,
			std::filesystem::copy_options::recursive
			| std::filesystem::copy_options::overwrite_existing);
#if _WIN32
		std::filesystem::copy("Tools/ProjectGenerator/WindowsFiles", ProjectPath,
			std::filesystem::copy_options::recursive
			| std::filesystem::copy_options::overwrite_existing);
#else
		std::filesystem::copy("Tools/ProjectGenerator/LinuxFiles", ProjectPath,
			std::filesystem::copy_options::recursive
			| std::filesystem::copy_options::overwrite_existing);
#endif
#if ENGINE_NO_SOURCE
		std::filesystem::copy("Tools/ProjectGenerator/ProjectFilesNoSource", ProjectPath,
			std::filesystem::copy_options::recursive
			| std::filesystem::copy_options::overwrite_existing);
#endif
#if ENGINE_NO_SOURCE
		if (LaunchArgs["ciBuild"] != "false")
#else
		if (LaunchArgs["buildSystem"] == "msvc")
#endif
		{
#if _WIN32
			for (auto& i : DependencyDlls)
			{

				if (!std::filesystem::exists(i))
				{
					std::cout << "Could not find " << i << ". Ensure you have the engine setup correctly." << std::endl;
					exit(1);
				}
				std::filesystem::copy(i, ProjectPath, std::filesystem::copy_options::overwrite_existing);
			}
#endif
		}
		else if (LaunchArgs["buildSystem"] == "cmake")
		{
#if _WIN32
			std::string NetHost = "CSharp/lib/nethost.dll";
#else
			std::string NetHost = "CSharp/lib/libnethost.so";
#endif
			std::filesystem::copy(NetHost, ProjectPath, std::filesystem::copy_options::overwrite_existing);
		}
	}

#if ENGINE_NO_SOURCE
	if (LaunchArgs["ciBuild"] == "false")
	{
		return 0;
	}
#endif

	const std::string& BuildSystem = LaunchArgs["buildSystem"];
	if (BuildSystem == "msbuild")
	{
		SLN::WriteMSVCProjectFiles(ProjectPath, ProjectName, LaunchArgs);
	}
	else
	{
		VSProj::WriteCSProj(ProjectPath + "/Scripts", "CSharpAssembly", LaunchArgs["netVersion"]);

		std::cout << "Copying files for build system: " << BuildSystem << std::endl;
		if (BuildSystem == "cmake")
		{
			std::ifstream in = std::ifstream("Tools/ProjectGenerator/BuildFiles/ProjectCMakeLists.txt");
			std::stringstream instr;
			instr << in.rdbuf();
			in.close();

			std::ofstream out = std::ofstream(ProjectPath + "/CMakeLists.txt");
			std::string OutString = instr.str();
			Util::ReplaceChar(OutString, '#', ProjectName);
			out << OutString;
			out.close();
		}
		else if (BuildSystem == "kbld")
		{
			std::filesystem::copy("Tools/ProjectGenerator/BuildFiles/makefile.kbld", ProjectPath + "/makefile.kbld");
		}
	}
}