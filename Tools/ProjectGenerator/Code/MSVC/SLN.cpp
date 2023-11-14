#include "SLN.h"
#include <fstream>
#include <filesystem>
#include <iostream>

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
		out << "Project(\"{" << GUIDTypes.at(i.Type) << "}\") = \"" << i.Name << + "\", ";
		if (i.Type != "folder")
		{
			out << "\"" << i.Path + "/" + i.Name + "." + i.Type << "\", ";
		}
		else
		{
			out << "\"" << i.Name << "\", ";
			Folders.push_back(i);
		}
		out << "\"{" << i.GUID << "}\"" << std::endl;

		if (i.Dependencies.size() && i.Type != "folder")
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
		if (i.Type == "vcxproj")
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
		else if (i.Type == "csproj")
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
