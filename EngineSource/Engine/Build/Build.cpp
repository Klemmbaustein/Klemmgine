#include "Build.h"
#include <filesystem>
#include <Engine/Log.h>
#include <Engine/EngineProperties.h>
#include <fstream>
#include <sstream>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Application.h>
#include <Engine/OS.h>

#if EDITOR
#include <Engine/Stats.h>
#include <iostream>
#include <Engine/Subsystem/CSharpInterop.h>
#include <Engine/Build/Pack.h>

namespace Build
{
	std::string GetSystemCommandReturnValue(const std::string& command)
	{
		int retval = system((command + " > temp.txt").c_str());

		std::ifstream ifs("temp.txt");
		std::string ret{ std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>() };
		ifs.close(); // must close the inout stream so the file can be cleaned up
		if (std::remove("temp.txt") != 0)
		{
			perror("Error deleting temporary file");
		}
		if (!ret.empty() && ret[ret.size() - 1] == '\n')
		{
			ret.pop_back();
		}
		return ret;
	}
#if _WIN32
	static std::string GetVSLocation()
	{
		auto test = GetSystemCommandReturnValue("cmd /C \"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\"\
 -latest -property installationPath");
		return test;
	}
#endif
}

std::string Build::TryBuildProject(std::string TargetFolder)
{
	try
	{
		if (std::filesystem::exists(TargetFolder))
		{
			int ret = 0;
			Log::Print("[Build]: -- Starting build --");
			const auto DirectoryCopyOptions = std::filesystem::copy_options::recursive;
			Stats::EngineStatus = "[Build]: Clearing folder";
			Log::Print("[Build]: Clearing folder");

			for (const auto& entry : std::filesystem::directory_iterator(TargetFolder))
				std::filesystem::remove_all(entry.path());
			Stats::EngineStatus = "Build: Copying .dll files";
			Log::Print("[Build]: Copying binary files");
#if _WIN32
			std::string BinaryPath = FileUtil::GetPath(OS::GetExecutableName());

			if (CMake::IsUsingCMake())
			{
				std::filesystem::copy(BinaryPath + "/SDL2.dll", TargetFolder + "SDL2.dll");
				std::filesystem::copy(BinaryPath + "/SDL2_net.dll", TargetFolder + "SDL2_net.dll");
			}
			else
			{
				std::filesystem::copy("SDL2.dll", TargetFolder + "SDL2.dll");
				std::filesystem::copy("SDL2_net.dll", TargetFolder + "SDL2_net.dll");
			}
			std::filesystem::copy(BinaryPath + "/OpenAL32.dll", TargetFolder + "/OpenAL32.dll");
#ifdef ENGINE_CSHARP
			std::filesystem::copy("nethost.dll", TargetFolder + "nethost.dll");
#endif
#endif
			Stats::EngineStatus = "Build: Creating folders";
			Log::Print("[Build]: Creating folders");

			std::filesystem::create_directories(TargetFolder + "/Assets/Content");

			Stats::EngineStatus = "Build: Packaging shaders";
			Log::Print("[Build]: Packaging shaders");
			Pack::SaveFolderToPack("Shaders/", TargetFolder + "/Assets/shaders.pack");

			Stats::EngineStatus = "Build: Copying assets";
			Log::Print("[Build]: Copying assets");
			std::filesystem::copy("Content/", TargetFolder + "Assets/Content", DirectoryCopyOptions);
			std::filesystem::copy("Fonts", TargetFolder + "Assets/");
			if (std::filesystem::exists("Locale"))
			{
				std::filesystem::copy("Locale", TargetFolder + "Assets/Locale");
			}
			Stats::EngineStatus = "Building C++ code";

#if ENGINE_NO_SOURCE
#if _WIN32
			std::filesystem::copy("bin/Klemmgine-Release.exe", TargetFolder + Project::ProjectName + std::string(".exe"));
#else
			std::filesystem::copy("bin/Release/Klemmgine-Release", TargetFolder + Project::ProjectName);
#endif
#else // ENGINE_NO_SOURCE

			if (CMake::IsUsingCMake())
			{
				CMake::BuildWithConfig("Release", "-DRELEASE=ON");

				std::string ExecutablePath = CMake::GetBuildRootPath("Release") + "/";

				std::string ExecutableExtension;

#if _WIN32
				ExecutablePath.append("Release/");
				ExecutableExtension = ".exe";
#endif

				std::filesystem::copy(ExecutablePath + Build::GetProjectBuildName() + ExecutableExtension, TargetFolder + Project::ProjectName + ExecutableExtension);
			}
			else
			{
#if _WIN32
				int CompileResult = BuildCurrentSolution("Release");
				if (!CompileResult)
				{
					std::filesystem::copy("bin/" + GetProjectBuildName() + "-Release.exe", TargetFolder + Project::ProjectName + std::string(".exe"));
				}
				else
				{
					Log::Print("[Build]: Failure: MSBuild returned " + std::to_string(CompileResult), Vector3(1, 0, 0));
					return "";
				}
#else // _WIN32
				Log::Print("[Build]: Running KlemmBuild...");
				ret = system("KlemmBuild -DRelease");

				if (ret)
				{
					Log::Print("[Build]: Compile failed", Log::LogColor::Red);
					return "";
				}

				for (auto& i : std::filesystem::directory_iterator("bin/Release"))
				{
					std::filesystem::copy(
						i,
						TargetFolder + "/" + i.path().filename().string(),
						std::filesystem::copy_options::overwrite_existing
					);
				}
				std::filesystem::rename(TargetFolder + "/Klemmgine-Release", TargetFolder + "/" + Project::ProjectName);
#endif // _WIN32

#endif // ENGINE_NO_SOURCE
		}
#if ENGINE_CSHARP
			if (CSharpInterop::GetUseCSharp())
			{
				std::filesystem::create_directories(TargetFolder + "bin");
				Log::Print("[Build]: Building C# core...");
				ret = system(("cd " + Application::GetEditorPath() + "/CSharp/Core && dotnet build").c_str());
				ret = system(("cd " + Application::GetEditorPath() + "/CSharp/Engine && dotnet build").c_str());

				std::filesystem::create_directories(TargetFolder + "/bin/CSharp/Core");
				std::filesystem::create_directories(TargetFolder + "/bin/CSharp/Engine");
				std::filesystem::copy(Application::GetEditorPath() + "/CSharp/Core/Build", TargetFolder + "/bin/CSharp/Core");
				std::filesystem::copy(Application::GetEditorPath() + "/CSharp/Engine/Build", TargetFolder + "/bin/CSharp/Engine");
				Log::Print("[Build]: Building game C# assembly...");
				ret = system("cd Scripts && dotnet build");

				std::filesystem::copy("CSharp/Build/", TargetFolder + "/bin/CSharp");

				std::string NetRuntimePath = GetSystemCommandReturnValue("dotnet --list-runtimes");

				std::vector<std::string> NetRuntimes = { "" };
				for (char c : NetRuntimePath)
				{
					if (c == '\n')
					{
						NetRuntimes.push_back("");
					}
					else
					{
						NetRuntimes[NetRuntimes.size() - 1].push_back(c);
					}
				}

				Log::Print("[Build]: Copying .net runtime. Available runtimes: ");

				std::string LatestRuntimePath;
				std::string LatestRuntimeVersion;

				for (std::string Runtime : NetRuntimes)
				{
					if (Runtime.substr(0, 21) != "Microsoft.NETCore.App")
					{
						continue;
					}
					Runtime = Runtime.substr(22);


					std::string VersionNumber = Runtime.substr(0, Runtime.find_first_of(" "));
					std::string Path = Runtime.substr(Runtime.find_first_of(" ") + 2);
					Path.pop_back();

					if (VersionNumber.substr(0, VersionNumber.find_first_of(".")) != CSharpInterop::GetNetVersion())
					{
						continue;
					}

					Log::Print("[Build]: \t" + Path + " : " + VersionNumber);
					LatestRuntimePath = Path + "/" + VersionNumber;
					LatestRuntimeVersion = VersionNumber;
				}
				std::string ToDir = TargetFolder + "/bin/NetRuntime/shared/Microsoft.NETCore.App/" + LatestRuntimeVersion;
				Log::Print("[Build]: Using .net runtime version " + LatestRuntimeVersion);
				std::filesystem::create_directories(ToDir);
				std::filesystem::copy(LatestRuntimePath, ToDir, DirectoryCopyOptions);

				std::string HostPath = TargetFolder + "/bin/NetRuntime/host/fxr/" + LatestRuntimeVersion;
				std::filesystem::create_directories(HostPath);
				std::filesystem::copy(LatestRuntimePath + "/../../../host/fxr/" + LatestRuntimeVersion, HostPath, DirectoryCopyOptions);
			}
#endif
			Log::Print("[Build]: Complete", Log::LogColor::Green);
			return "Success";
		}
		Log::Print("[Build]: Cannot find folder '" + TargetFolder + "' - Creating...", Log::LogColor::Yellow);
		std::filesystem::create_directories(TargetFolder);
		return Build::TryBuildProject(TargetFolder);
	}
	catch (std::exception& e)
	{
		Log::Print(std::string("Build: Failure: Exception thrown: ") + e.what(), Vector3(1, 0, 0));
		return "Error";
	}
}
int Build::BuildCurrentSolution(std::string Configuration)
{
#if _WIN32
	std::string VSLocation = GetVSLocation();
	std::string MSBuildPath = "\"" + VSLocation + "\\MSBuild\\Current\\Bin\\msbuild.exe\"";
#else
	std::string MSBuildPath = "dotnet msbuild";
#endif
	std::string SolutionName;
	for (auto& i : std::filesystem::directory_iterator("."))
	{
		std::string SolutionString = i.path().string();
		if (SolutionString.substr(SolutionString.find_last_of(".")) == ".sln")
		{
			SolutionName = FileUtil::GetFileNameWithoutExtensionFromPath(SolutionString);
		}
	}
	if (SolutionName.empty())
	{
		Log::Print("[Build]: Attempted to build project solution but there is no .sln file in the main folder", Vector3(1, 0, 0));
		return 1;
	}
	Log::Print("[Build]: Found .sln file: " + SolutionName, Log::LogColor::Green);

	std::string Command = (MSBuildPath + " " + SolutionName + ".sln /p:Configuration=" + Configuration);

	std::cout << Command << std::endl;

	Log::Print("[Build]: Invoking MSBuild - " + MSBuildPath, Vector3(1));
	int ret = system(Command.c_str());
	if (!ret)
	{
		Log::Print("[Build]: Built project for configuration: " + Configuration, Log::LogColor::Green);
	}

	return ret;
}
#endif


#if !RELEASE


std::string Build::GetProjectBuildName()
{
	std::string Name = GetSolutionName();

	if (Name.empty())
	{
		Name = FileUtil::GetFileNameWithoutExtensionFromPath(OS::GetExecutableName());
	}
	return Name;
}

std::string Build::GetSolutionName()
{
	std::string ProjectName;
	for (auto& i : std::filesystem::directory_iterator("."))
	{
		std::string SolutionString = i.path().string();
		if (SolutionString.substr(SolutionString.find_last_of(".")) == ".sln")
		{
			ProjectName = FileUtil::GetFileNameWithoutExtensionFromPath(SolutionString);
		}
	}
	return ProjectName;
}


std::string Build::CMake::GetBuildRootPath(std::string Configuration)
{
	// (More or less) match the Visual Studio default cmake build root (${projectDir}\out\build\${name})
#if _WIN32
	return "out\\engine-build\\" + Configuration;
#else
	return "out/engine-build/" + Configuration;
#endif
}

static std::string CMakeMSBuildConfig = "Release";

bool Build::CMake::BuildWithConfig(std::string Configuration, std::string Args)
{
	std::string RootPath = GetBuildRootPath(Configuration);

	if (!std::filesystem::exists(RootPath) && system(("cmake -S . -B " + RootPath + " " + Args).c_str()))
	{
		return false;
	}

#if _WIN32
	return system(("cmake --build " + RootPath + " --config " + CMakeMSBuildConfig + " -- /m:2 /p:CL_MPCount=12 ").c_str()) == 0;
#else
	return system(("cmake --build " + RootPath).c_str());
#endif
}

bool Build::CMake::IsUsingCMake()
{
	for (const auto& file : std::filesystem::directory_iterator("."))
	{
		if (file.path().filename().u8string() == u8"CMakeLists.txt")
		{
			return true;
		}
	}
	return false;
}

void Build::CMake::SetMSBuildConfig(std::string Name)
{
	CMakeMSBuildConfig = Name;
}

std::string Build::CMake::GetMSBuildConfig()
{
	return CMakeMSBuildConfig;
}
#endif
