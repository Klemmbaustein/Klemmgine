#include "VSProj.h"

#include "../XML/XML.h"
#include "../Util.h"

#include <fstream>
#include <filesystem>
#include "SLN.h"
#include <iostream>
#include <algorithm>

int Run(std::string Command)
{
	return system(Command.c_str());
}

std::string VSProj::WriteVCXProj(std::string Path, std::string Name, std::string WinSdkVer, std::string PlatformToolset, bool WithBuildTool)
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
		CurrentPath.string() + "/EngineSource",
		"../GeneratedIncludes",
		CurrentPath.string() + "/Dependencies/glm",
	};

	std::vector<std::string> LibraryPaths =
	{
		CurrentPath.string() + "/lib",
		CurrentPath.string() + "/CSharp/lib",
		CurrentPath.string() + "/Dependencies/glew-cmake/lib/Release",
		CurrentPath.string() + "/Dependencies/assimp/lib/Release",
		CurrentPath.string() + "/Dependencies/openal-soft/Release",
		CurrentPath.string() + "/Dependencies/SDL/VisualC/SDL/x64/Release",
		CurrentPath.string() + "/Dependencies/SDL_net/Build/Release",
		CurrentPath.string() + "/Dependencies/JoltPhysics/Build/VS2022_CL/Distribution"

	};

	std::vector<std::string> Configurations =
	{
		"Debug",
		"Editor",
		"Release",
		"Server"
	};
	
	std::string IncludePathsString;

	for (auto& i : IncludePaths)
	{
		if (i[1] != ':')
		{
			IncludePathsString.append("$(ProjectDir)" + i + ";");
		}
		else
		{
			IncludePathsString.append(i + ";");
		}
	}
	IncludePathsString.append("$(IncludePath)");

	std::string LibraryPathsString;

	for (auto& i : LibraryPaths)
	{
		LibraryPathsString.append(i + ";");
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

	/*
	*   <OutDir>$(ProjectDir)..\</OutDir>
	*	<TargetName>$(ProjectName)-$(Configuration)</TargetName>
	*/

	Project.AddTag("DefaultTargets", "Build")
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
			.Add(XML("ProjectGuid", "{" + GUID + "}"))
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

	Project.Add(XML("PropertyGroup")
		.Add(XML("OutDir", "$(ProjectDir)..\\bin\\"))
		.Add(XML("TargetName", "$(ProjectName)-$(Configuration)")));

	for (auto& i : Configurations)
	{
		std::string UpperCaseName = i;
		std::transform(UpperCaseName.begin(), UpperCaseName.end(), UpperCaseName.begin(), ::toupper);

		auto DefGroup = XML("ItemDefinitionGroup");
		DefGroup
			.AddTag("Condition", "'$(Configuration)|$(Platform)'=='" + i + "|x64'")
			.Add(XML("ClCompile")
				.Add(XML("LanguageStandard", "stdcpp20"))
				.Add(XML("LanguageStandard_C", "stdc17"))
				.Add(XML("MultiProcessorCompilation", "true"))
				.Add(XML("PreprocessorDefinitions", "$(ExternalCompilerOptions);%(PreprocessorDefinitions)"))
				.Add(XML("PreprocessorDefinitions", UpperCaseName + ";NDEBUG;_CONSOLE;GLEW_STATIC;ENGINE_CSHARP;%(PreprocessorDefinitions)"))
				.Add(XML("ObjectFileName", "..\\$(IntDir)"))
				.Add(XML("AdditionalOptions", "%(AdditionalOptions)")))
			.Add(XML("Link")
				.Add(XML("OptimizeReferences", "true"))
				.Add(XML("EnableCOMDATFolding", "true"))
				.Add(XML("FavorSizeOrSpeed", "Speed"))
				.Add(XML("AdditionalDependencies",
					"assimp-vc143-mt.lib;OpenAL32.lib;SDL2_net.lib;SDL2.lib;opengl32.lib;glew.lib;Engine-$(Configuration).lib;nethost.lib;Jolt.lib;%(AdditionalDependencies)"))
				.Add(XML("OutputFile", "$(OutputPath)$(TargetName)$(TargetExt)"))
				.Add(XML("Subsystem", "Console")));

		if (WithBuildTool)
		{
			DefGroup.Add(XML("PreBuildEvent")
				.Add(XML("Command",
					"\""
					+ CurrentPath.string()
					+ "/Tools/bin/BuildTool.exe\" in="
					+ CurrentPath.string()
					+ "/EngineSource/Objects in=./Objects out=../GeneratedIncludes")));
		}

		Project.Add(DefGroup);
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
			.Add(XML("LocalDebuggerCommandArguments", "-editorPath " + CurrentPath.string()))
			.Add(XML("LocalDebuggerCommand", "$(ProjectDir)../bin/$(ProjectName)-$(Configuration)$(TargetExt)"))
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
	std::cout << "Writing " << Name << ".csproj for .NET version " << TargetFramework << std::endl;
	std::filesystem::path CurrentPath = std::filesystem::current_path();

	std::filesystem::create_directories(Path);
	std::filesystem::current_path(Path);
	XML Project = XML("Project");
	/*
	*  <ItemGroup>
	*    <Reference Include="KlemmgineCSharp.dll">
	*      <HintPath>../../../CSharp/Assembly/Build/KlemmgineCSharp.dll</HintPath>
	*    </Reference>
	*  </ItemGroup>
	*/
	Project.AddTag("Sdk", "Microsoft.NET.Sdk")
		.Add(XML("PropertyGroup")
			.Add(XML("TargetFramework", TargetFramework))
			.Add(XML("EnableDynamicLoading", "true")))
		.Add(XML("PropertyGroup")
			.Add(XML("OutputPath", "../CSharp/Build"))
			.Add(XML("AppendTargetFrameworkToOutputPath", "false")))
		.Add(XML("ItemGroup")
			.Add(XML("Reference").AddTag("Include", "KlemmgineCSharp.dll")
			.Add(XML("HintPath", CurrentPath.string() + "/CSharp/Engine/Build/KlemmgineCSharp.dll"))));

	std::ofstream out = std::ofstream(Name + ".csproj");
	out << Project.Write();
	out.close();

	std::filesystem::current_path(CurrentPath);
	std::string GUID = SLN::GetGUID();
	std::cout << "Finished writing csproj - " << GUID << std::endl << std::endl;
	return GUID;
}
