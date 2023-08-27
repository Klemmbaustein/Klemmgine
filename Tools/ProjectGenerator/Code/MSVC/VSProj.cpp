#include "VSProj.h"

#include "../XML/XML.h"
#include "../Util.h"

#include <fstream>
#include <filesystem>
#include "SLN.h"
#include <iostream>

int Run(std::string Command)
{
	return system(Command.c_str());
}

std::string VSProj::WriteVCXProj(std::string Path, std::string Name, std::string WinSdkVer, std::string PlatformToolset)
{
	std::cout << "Writing " << Name << ".vcxproj for windows SDK version " << WinSdkVer << " and platform toolset version " << PlatformToolset << std::endl;

	std::string GUID = SLN::GetGUID();
	std::filesystem::path CurrentPath = std::filesystem::current_path();

	std::filesystem::create_directories(Path);
	std::filesystem::current_path(Path);
	XML Project = XML("Project");

	XML ClCompiles = XML("ItemGroup");
	XML ClIncludes = XML("ItemGroup");
	XML NoneFiles = XML("ItemGroup");

	std::vector<std::string> IncludePaths =
	{
		"./",
		"../../../EngineSource",
		"../GeneratedIncludes",
		"../../../Dependencies/glm",
		"../../../Dependencies/SDL/include"
	};

	std::vector<std::string> LibraryPaths =
	{
		"../../../lib",
		"../../../CSharpCore/lib",
		"../../../Dependencies/glew-cmake/lib/Release/x64",
		"../../../Dependencies/assimp/lib/Release",
		"../../../Dependencies/openal-soft/Release",
		"../../../Dependencies/SDL/VisualC/SDL/x64/Release"

	};

	std::vector<std::string> Configurations =
	{
		"Debug",
		"Editor",
		"Release"
	};
	
	std::string IncludePathsString;

	for (auto& i : IncludePaths)
	{
		IncludePathsString.append("$(ProjectDir)" + i + ";");
	}
	IncludePathsString.append("$(IncludePath)");

	std::string LibraryPathsString;

	for (auto& i : LibraryPaths)
	{
		LibraryPathsString.append("$(ProjectDir)" + i + ";");
	}
	LibraryPathsString.append("$(LibraryPath)");

	auto Files = Util::GetAllFilesInFolder("./", false);
	for (auto& File : Files)
	{
		if (Util::GetExtension(File) == "cpp")
		{
			ClCompiles.Add(XML("ClCompile")
				.AddTag("Include", "." + File));
		}
		else if (Util::GetExtension(File) == "hpp" || Util::GetExtension(File) == "h")
		{
			ClIncludes.Add(XML("ClInclude")
				.AddTag("Include", "." + File));
		}
		else if (Util::GetExtension(File) == "vert" || Util::GetExtension(File) == "frag" || Util::GetExtension(File) == "geom")
		{
			NoneFiles.Add(XML("None")
				.AddTag("Include", "." + File));
		}
	}

	Project.AddTag("DefaultTargets", Name)
		.AddTag("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

	XML ProjectConfigGroup = XML("ItemGroup")
		.AddTag("Label", "ProjectConfigurations");

	for (auto& i : Configurations)
	{
		ProjectConfigGroup
			.Add(XML("ProjectConfiguration")
				.AddTag("Include", i + "|x64")
				.Add(XML("Configuration", i))
				.Add(XML("Platform", "x64")));
	}

	Project.Add(ProjectConfigGroup)
		.Add(XML("PropertyGroup")
			.AddTag("Label", "Globals")
			.Add(XML("ShowAllFiles", "true"))
			.Add(XML("VCProjectVersion", "16.0"))
			.Add(XML("WindowsTargetPlatformVersion", WinSdkVer))
			.Add(XML("ProjectGuid", GUID))
			.Add(XML("Keyword", "Win32Proj"))
			.Add(XML("PlatformToolset", PlatformToolset)))
		.Add(XML("Import")
			.AddTag("Project", "$(VCTargetsPath)\\Microsoft.Cpp.default.props"))
		.Add(XML("Import")
			.AddTag("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props"))
		.Add(XML("PropertyGroup")
			.Add(XML("IncludePath", IncludePathsString))
			.Add(XML("LibraryPath", LibraryPathsString)))

		.Add(NoneFiles)
		.Add(ClCompiles)
		.Add(ClIncludes);

	for (auto& i : Configurations)
	{
		std::string UpperCaseName = i;
		std::transform(UpperCaseName.begin(), UpperCaseName.end(), UpperCaseName.begin(), ::toupper);

		Project.Add(XML("ItemDefinitionGroup")
			.AddTag("Condition", "'$(Configuration)|$(Platform)'=='" + i + "|x64'")
			.Add(XML("ClCompile")
				.Add(XML("LanguageStandard", "stdcpp20"))
				.Add(XML("LanguageStandard_C", "stdc17"))
				.Add(XML("MultiProcessorCompilation", "true"))
				.Add(XML("PreprocessorDefinitions", UpperCaseName + ";NDEBUG;_CONSOLE;GLEW_STATIC;ENGINE_CSHARP;%(PreprocessorDefinitions)"))
				.Add(XML("ObjectFileName", "..\\$(IntDir)")))
			.Add(XML("Link")
				.Add(XML("OptimizeReferences", "true"))
				.Add(XML("EnableCOMDATFolding", "true"))
				.Add(XML("FavorSizeOrSpeed", "Speed"))
				.Add(XML("AdditionalDependencies",
					"assimp-vc143-mt.lib;OpenAL32.lib;SDL2.lib;opengl32.lib;glew32s.lib;Engine-$(Configuration).lib;nethost.lib;%(AdditionalDependencies)"))
				.Add(XML("OutputFile", "$(ProjectDir)../$(ProjectName)-$(Configuration)$(TargetExt)"))
				.Add(XML("Subsystem", "Console")))
			.Add(XML("PreBuildEvent")
				.Add(XML("Command",
					"$(ProjectDir)../../../Tools/bin/BuildTool.exe in=../../../EngineSource/Objects in=./Objects out=../GeneratedIncludes"))));
	}

	Project.Add(XML("Import")
		.AddTag("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets"));

	std::ofstream out = std::ofstream(Name + ".vcxproj");
	out << Project.Write();
	out.close();
	std::cout << "Writing .vcxproj.user file" << std::endl;
	XML User = XML("Project")
		.AddTag("ToolsVersion", "Current")
		.AddTag("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003")
		.Add(XML("PropertyGroup")
			.Add(XML("LocalDebuggerWorkingDirectory", "$(ProjectDir)../"))
			.Add(XML("LocalDebuggerCommand", "$(ProjectDir)../$(ProjectName)-$(Configuration)$(TargetExt)"))
			.Add(XML("DebuggerFlavor", "WindowsLocalDebugger")));
	std::ofstream UserOut = std::ofstream(Name + ".vcxproj.user");
	UserOut << User.Write();
	UserOut.close();

	std::filesystem::current_path(CurrentPath);
	std::cout << "Finished writing vcxproj - " << GUID << std::endl << std::endl;
	

	return GUID;
}

std::string VSProj::WriteCSProj(std::string Path, std::string Name, std::string TargetFramework)
{
	std::cout << "Writing " << Name << ".csproj for framework version " << TargetFramework << std::endl;
	std::filesystem::path CurrentPath = std::filesystem::current_path();

	std::filesystem::create_directories(Path);
	std::filesystem::current_path(Path);
	XML Project = XML("Project");

	Project.AddTag("Sdk", "Microsoft.NET.Sdk")
		.Add(XML("PropertyGroup")
			.Add(XML("TargetFramework", TargetFramework))
			.Add(XML("EnableDynamicLoading", "true")))
		.Add(XML("PropertyGroup")
			.Add(XML("OutputPath", "../CSharp/Build"))
			.Add(XML("AppendTargetFrameworkToOutputPath", "false")));

	std::ofstream out = std::ofstream(Name + ".csproj");
	out << Project.Write();
	out.close();

	std::filesystem::current_path(CurrentPath);
	std::string GUID = SLN::GetGUID();
	std::cout << "Finished writing csproj - " << GUID << std::endl << std::endl;
	return GUID;
}
