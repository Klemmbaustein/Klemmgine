#include "Build.h"
#include <filesystem>
#include <Engine/Log.h>
#include <Engine/EngineProperties.h>
#include <fstream>
#include <sstream>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Application.h>

#if EDITOR
#include <Engine/Stats.h>
#include <iostream>
#include <Engine/Subsystem/CSharpInterop.h>
#include <Engine/Build/Pack.h>

namespace Build
{
	std::string GetSystemCommandReturnValue(const std::string& command)
	{
		std::system((command + " > temp.txt").c_str());

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

#if _WIN32

#define NOMINMAX
#include <Windows.h>

LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue, const std::wstring& strDefaultValue)
{
	strValue = strDefaultValue;
	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		strValue = szBuffer;
	}
	return nError;
}

#endif

std::string Build::TryBuildProject(std::string TargetFolder)
{
	try
	{
		if (std::filesystem::exists(TargetFolder))
		{
			Log::Print("[Build]: -- Starting build --");
			const auto DirectoryCopyOptions = std::filesystem::copy_options::recursive;
			Debugging::EngineStatus = "[Build]: Clearing folder";
			Log::Print("[Build]: Clearing folder");

			for (const auto& entry : std::filesystem::directory_iterator(TargetFolder))
				std::filesystem::remove_all(entry.path());
			std::filesystem::create_directories(TargetFolder + "bin");
			Debugging::EngineStatus = "Build: Copying .dll files";
			Log::Print("[Build]: Copying .dll files");
#if _WIN32
			std::filesystem::copy("SDL2.dll", TargetFolder + "SDL2.dll");
			std::filesystem::copy("SDL2_net.dll", TargetFolder + "SDL2_net.dll");
			std::filesystem::copy("bin/OpenAL32.dll", TargetFolder + "bin/OpenAL32.dll");
#ifdef ENGINE_CSHARP
			std::filesystem::copy("nethost.dll", TargetFolder + "nethost.dll");
#endif
#else
			std::filesystem::copy("bin/libSDL2_net.so", TargetFolder + "/libSDL2_net.so");
#ifdef ENGINE_CSHARP
			std::filesystem::copy("bin/libnethost.so", TargetFolder + "/libnethost.so");
#endif
#endif
			Debugging::EngineStatus = "Build: Creating folders";
			Log::Print("[Build]: Creating folders");

			std::filesystem::create_directories(TargetFolder + "/Assets/Content");

			Debugging::EngineStatus = "Build: Packaging shaders";
			Log::Print("[Build]: Packaging shaders");
			Pack::SaveFolderToPack("Shaders/", TargetFolder + "/Assets/shaders.pack");

			Debugging::EngineStatus = "Build: Copying assets";
			Log::Print("[Build]: Copying assets");
			std::filesystem::copy("Content/", TargetFolder + "Assets/Content", DirectoryCopyOptions);
			std::filesystem::copy("Fonts", TargetFolder + "Assets/");
			if (std::filesystem::exists("Locale"))
			{
				std::filesystem::copy("Locale", TargetFolder + "Assets/Locale");
			}
			Debugging::EngineStatus = "Building C++ solution";
#if _WIN32

#if ENGINE_NO_SOURCE
			std::filesystem::copy("bin/Klemmgine-Release.exe", TargetFolder + Project::ProjectName + std::string(".exe"));
#else
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
#endif
#else
#if !ENGINE_NO_SOURCE
			Log::Print("[Build]: Running KlemmBuild...");
			system("KlemmBuild -DRelease");
#endif
			std::filesystem::copy("bin/Klemmgine-Release", TargetFolder + Project::ProjectName);
#endif
#if ENGINE_CSHARP
			if (CSharpInterop::GetUseCSharp())
			{
				Log::Print("[Build]: Building C# core...");
				system(("cd " + Application::GetEditorPath() + "/CSharp/Core && dotnet build").c_str());
				system(("cd " + Application::GetEditorPath() + "/CSharp/Engine && dotnet build").c_str());

				std::filesystem::create_directories(TargetFolder + "/bin/CSharp/Core");
				std::filesystem::create_directories(TargetFolder + "/bin/CSharp/Engine");
				std::filesystem::copy(Application::GetEditorPath() + "/CSharp/Core/Build", TargetFolder + "/bin/CSharp/Core");
				std::filesystem::copy(Application::GetEditorPath() + "/CSharp/Engine/Build", TargetFolder + "/bin/CSharp/Engine");
				Log::Print("[Build]: Building game C# assembly...");
				system("cd Scripts && dotnet build");

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

				Log::Print("[Build]: Copying .net runtime. Avaliable runtimes: ");

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
		Log::Print("[Build]: Build system is 'msvc' but there is no .sln file in the main folder", Vector3(1, 0, 0));
		return 1;
	}
	Log::Print("[Build]: Found .sln file: " + SolutionName, Log::LogColor::Green);

	std::filesystem::create_directories("Build\\");

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

#endif