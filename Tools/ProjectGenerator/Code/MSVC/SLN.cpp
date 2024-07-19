#include "SLN.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include "VSProj.h"

size_t iterator = 0;
std::vector<std::string> KnownValidGUIDs =
{
	"53B4FA76-7692-4CBC-933A-275FB31CADB6",
	"196997C4-DCE6-449E-ACC1-3BD7756003A3",
	"5B7656A8-A442-4EFA-B737-97B9CA169A78",
	"7313A2E3-68BA-44D6-85F5-FFE2DF28914B",
	"9835F0E7-CFA4-41D3-8D33-5656B8D21C6B",
};
std::string SLN::GetGUID()
{
	return KnownValidGUIDs.at(iterator++);
}

void SLN::WriteSolution(std::string Path, std::string Name, std::vector<Project> Projects)
{
	std::ofstream out = std::ofstream(Path + "/" + Name + ".sln");
	out << "Microsoft Visual Studio Solution File, Format Version 12.00\n\
# Visual Studio Version 17\n\
VisualStudioVersion = 17.2.32505.173\n\
MinimumVisualStudioVersion = 10.0.40219.1\n";

	std::vector<Project> Folders;

	for (const auto& i : Projects)
	{
		out << "Project(\"{" << GUIDTypes.at(i.NativeType) << "}\") = \"" << i.Name << + "\", ";
		if (i.NativeType != "folder")
		{
			out << "\"" << i.Path + "/" + i.Name + "." + i.NativeType << "\", ";
		}
		else
		{
			out << "\"" << i.Name << "\", ";
			Folders.push_back(i);
		}
		out << "\"{" << i.GUID << "}\"" << std::endl;

		if (i.Dependencies.size() && i.NativeType != "folder")
		{
			out << "	ProjectSection(ProjectDependencies) = postProject" << std::endl;
			for (auto& dep : i.Dependencies)
			{
				out << "		{" << dep << "} = {" << dep << "}" << std::endl;
			}
			out << "	EndProjectSection" << std::endl;
		}

		out << "EndProject" << std::endl;
	}

	out << "Global\n\
	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n\
		Debug|x64 = Debug|x64\n\
		Editor|x64 = Editor|x64\n\
		Release|x64 = Release|x64\n\
		Server|x64 = Server|x64\n\
	EndGlobalSection\n";
	out << "	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n";

	for (auto& i : Projects)
	{
		if (i.NativeType == "vcxproj")
		{
			out << "		{" + i.GUID + "}.Debug|x64.ActiveCfg = Debug|x64\n\
		{" + i.GUID + "}.Debug|x64.Build.0 = Debug|x64\n\
		{" + i.GUID + "}.Editor|x64.ActiveCfg = Editor|x64\n\
		{" + i.GUID + "}.Editor|x64.Build.0 = Editor|x64\n\
		{" + i.GUID + "}.Server|x64.ActiveCfg = Server|x64\n\
		{" + i.GUID + "}.Server|x64.Build.0 = Server|x64\n\
		{" + i.GUID + "}.Release|x64.ActiveCfg = Release|x64\n\
		{" + i.GUID + "}.Release|x64.Build.0 = Release|x64\n";
		}
		else if (i.NativeType == "csproj")
		{
			out << "		{" + i.GUID + "}.Debug|x64.ActiveCfg = Release|Any CPU\n\
		{" + i.GUID + "}.Debug|x64.Build.0 = Release|Any CPU\n\
		{" + i.GUID + "}.Editor|x64.ActiveCfg = Release|Any CPU\n\
		{" + i.GUID + "}.Editor|x64.Build.0 = Release|Any CPU\n\
		{" + i.GUID + "}.Server|x64.ActiveCfg = Release|Any CPU\n\
		{" + i.GUID + "}.Server|x64.Build.0 = Release|Any CPU\n\
		{" + i.GUID + "}.Release|x64.ActiveCfg = Release|Any CPU\n\
		{" + i.GUID + "}.Release|x64.Build.0 = Release|Any CPU\n";
		}
	}
	out << "	EndGlobalSection\n";

	out << "	GlobalSection(SolutionProperties) = preSolution\n\
		HideSolutionNode = FALSE\n\
	EndGlobalSection\n";

	if (Folders.size())
	{
		out << "	GlobalSection(NestedProjects) = preSolution\n";
		for (const auto& i : Folders)
		{
			for (const auto j : i.Dependencies)
			{
				out << "		{" << j << "} = {" + i.GUID << "}\n";
			}
		}
		out << "	EndGlobalSection\n";
	}

	out << "EndGlobal";
	out.close();
}

void SLN::WriteMSVCProjectFiles(std::string Path, std::string Name, std::map<std::string, std::string> LaunchArgs)
{
	std::string CppGUID = VSProj::WriteVCXProj(Path + "/Code", Name, "10.0", "v143", true);

	std::cout << "Generating solution..." << std::endl;

	std::vector<SLN::Project> Projects;

#if ENGINE_NO_SOURCE || __linux__
	if (LaunchArgs["ciBuild"] != "false")
#endif
	{
		SLN::Project CppProject;
		CppProject.Name = Name;
		CppProject.Path = "Code";
		CppProject.GUID = "A2BEFDE1-9019-4A47-839E-545ACCF559F2";
		CppProject.NativeType = "vcxproj";
		Projects.push_back(CppProject);
	}
	if (LaunchArgs["includeCsharp"] == "true")
	{
		std::cout << "- Including C# project in solution" << std::endl;
		std::string CsGUID = VSProj::WriteCSProj(Path + "/Scripts", "CSharpAssembly", LaunchArgs["netVersion"]);

		SLN::Project CSProject;
		CSProject.Name = "CSharpAssembly";
		CSProject.Path = "Scripts";
		CSProject.GUID = CsGUID;
		CSProject.NativeType = "csproj";
#if !ENGINE_NO_SOURCE && !__linux__
		Projects[0].Dependencies.push_back(CsGUID);
#endif
		Projects.push_back(CSProject);
	}

#if !ENGINE_NO_SOURCE && !__linux__
	if (LaunchArgs["includeEngine"] == "true")
	{
		std::cout << "- Including engine project in solution" << std::endl;
		SLN::Project EngineProject;
		EngineProject.Name = "Engine";
		EngineProject.NativeType = "vcxproj";
		EngineProject.Path = std::filesystem::current_path().append("EngineSource").string();
		EngineProject.GUID = "E25491B8-04A8-4B57-9B45-C73718142C84";
		Projects.push_back(EngineProject);

		SLN::Project EngineFolder;
		EngineFolder.GUID = "5B7656A8-A442-4EFA-B737-97B9CA169A78";
		EngineFolder.Name = "Klemmgine";
		EngineFolder.Dependencies = { EngineProject.GUID };
		EngineFolder.NativeType = "folder";
		Projects.push_back(EngineFolder);
		Projects[0].Dependencies.push_back(EngineProject.GUID);
	}
#endif

	std::string ShaderGUID = VSProj::WriteVCXProj(Path + "/Shaders", "Shaders", "10.0", "v143", false);
#if 1
	SLN::Project ShaderProject;
	ShaderProject.Name = "Shaders";
	ShaderProject.Path = "Shaders";
	ShaderProject.GUID = "91315EA7-5B25-4D8E-A732-C66EB73E510E";
	ShaderProject.NativeType = "vcxproj";
	Projects.push_back(ShaderProject);
#endif

	if (LaunchArgs["onlyBuildFiles"] == "false")
	{
		std::cout << "- Writing solution" << std::endl;
		SLN::WriteSolution(Path, Name, Projects);
		std::cout << "- Finished writing solution: " << Path << "/" << Name << ".sln" << std::endl;
	}
}
