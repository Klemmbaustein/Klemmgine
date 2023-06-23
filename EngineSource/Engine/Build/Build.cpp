#if EDITOR
#include "Build.h"
#include <filesystem>
#include <Engine/Build/Pack.h>
#include <Engine/Log.h>
#include <Engine/EngineProperties.h>
#include <fstream>
#include <sstream>
#include <World/Stats.h>
#include <Engine/FileUtility.h>
#include <iostream>

#if _WIN32
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
		ret.erase(std::remove(ret.begin(), ret.end(), '\n'), ret.cend()); //remove newline from the end of the file
		return ret;
	}
	std::string GetVSLocation()
	{
		auto test = GetSystemCommandReturnValue("cmd /C \"%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\"\
 -latest -property installationPath");
		return test;
	}
}

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
			Debugging::EngineStatus = "[Build]: Clearing folder";
			Log::Print("[Build]: Clearing folder");

			for (const auto& entry : std::filesystem::directory_iterator(TargetFolder))
				std::filesystem::remove_all(entry.path());

			Debugging::EngineStatus = "Build: Copying .dll files";
			Log::Print("[Build]: Copying .dll files");

			std::filesystem::copy("SDL2.dll", TargetFolder + "SDL2.dll");
			std::filesystem::copy("OpenAL32.dll", TargetFolder + "OpenAL32.dll");

			Debugging::EngineStatus = "Build: Creating folders";
			Log::Print("[Build]: Creating folders");

			std::filesystem::create_directories(TargetFolder + "/Assets/Content");

			Debugging::EngineStatus = "Build: Packaging shaders";
			Log::Print("[Build]: Packaging shaders");
			Pack::SaveFolderToPack("Shaders/", TargetFolder + "/Assets/shaders.pack");

			Debugging::EngineStatus = "Build: Copying assets";
			Log::Print("[Build]: Copying assets");
			const auto copyOptions = std::filesystem::copy_options::recursive;
			std::filesystem::copy("Content", TargetFolder + "/Assets/Content", copyOptions);
			std::filesystem::copy("Fonts", TargetFolder + "Assets/");
			Debugging::EngineStatus = "Building C++ solution";
#if _WIN32

			std::string VSInstallPath = GetVSLocation() + "\\Common7\\IDE\\devenv.exe";
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
				return "There is no MSBuild";
			}
			Log::Print("[Build]: Found .sln file: " + SolutionName, Vector3(0.5, 1, 0.5));

			std::string Command = "\"" + VSInstallPath + "\" " + std::string(SolutionName) + ".sln /Build Release";

			Log::Print("[Build]: Running command: " + Command + " (This can take a while)", Vector3(0.5));
			int CompileResult = system(Command.c_str());
			if(!CompileResult)
			{
				std::filesystem::copy("x64/Release/" + SolutionName + ".exe", TargetFolder + ProjectName + std::string(".exe"));
			}
			else
			{
				Log::Print("[Build]: Failure: MSBuild returned " + std::to_string(CompileResult), Vector3(1, 0, 0));
				return "MSBuild Failure";
			}
#else
			Log::Print("Build: Compiling is currently not supported on Linux.", Vector3(1, 0, 0));
			Log::Print("Pleasse recompile the program manually with the RELASE preprocessor definition (Release config).", Vector3(1, 0, 0));
#endif
			Log::Print("[Build]: Complete", Vector3(0, 1, 0));
			return "Sucess";
		}
		Log::Print("[Build]: Cannot find folder", Vector3(1, 0, 0));
		std::filesystem::create_directories(TargetFolder);
		return Build::TryBuildProject(TargetFolder);
	}
	catch (std::exception& e)
	{
		Log::Print(std::string("Build: Failure: Exception thrown: ") + e.what(), Vector3(1, 0, 0));
		return "Error";
	}
}
#endif