#include "MSVC/VSProj.h"
#include "MSVC/SLN.h"
#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>

std::vector<std::string> DependenyDlls =
{
	"Dependencies/assimp/bin/Release/assimp-vc143-mt.dll",
	"Dependencies/openal-soft/build/Release/OpenAL32.dll",
	"Dependencies/SDL/VisualC/SDL/x64/Release/SDL2.dll",
	"CSharpCore/lib/nethost.dll"
};

int main(int argc, char** argv)
{
	std::map<std::string, std::string> LaunchArgs =
	{
		std::pair("winSdk", "10.0"),
		std::pair("toolset", "v143"),
		std::pair("projectName", ""),
		std::pair("includeEngine", "true"),
		std::pair("includeCsharp", "true")
	};

	std::string CurrentArg = "";
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
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
		}
	}

	std::string ProjectName = LaunchArgs["projectName"];

	if (ProjectName.empty())
	{
		std::cout << "Expected project name. Make sure you passed a project name to the tool like this:\n" << argv[0] << " -projectName MyProject" << std::endl;
		return 1;
	}

	if (std::filesystem::exists("Games/" + ProjectName))
	{
		std::cout << "Warning: Games/" << ProjectName << "Already exists. Replacing..." << std::endl;
		std::filesystem::remove_all("Games/" + ProjectName);
	}

	std::cout << "Copying project files" << std::endl;
	std::filesystem::create_directories("Games/" + ProjectName + "/Code/Objects");
	std::filesystem::copy("Tools/ProjectGenerator/DefaultProjectFiles", "Games/" + ProjectName, std::filesystem::copy_options::recursive);

	for (auto& i : DependenyDlls)
	{
		if (!std::filesystem::exists(i))
		{
			std::cout << "Could not find " << i << ". Ensure you have the project setup correctly." << std::endl;
			exit(1);
		}
		std::filesystem::copy(i, "Games/" + ProjectName);
	}

	std::string CppGUID = VSProj::WriteVCXProj("Games/" + ProjectName + "/Code", ProjectName, "10.0", "v143");
	std::string ShaderGUID = VSProj::WriteVCXProj("Games/" + ProjectName + "/Shaders", "Shaders", "10.0", "v143");

	std::cout << "Generating solution..." << std::endl;

	std::vector<SLN::Project> Projects;

	SLN::Project CppProject;
	CppProject.Name = ProjectName;
	CppProject.Path = "Code";
	CppProject.GUID = CppGUID;
	CppProject.Type = "vcxproj";

	SLN::Project ShaderProject;
	ShaderProject.Name = "Shaders";
	ShaderProject.Path = "Shaders";
	ShaderProject.GUID = ShaderGUID;
	ShaderProject.Type = "vcxproj";

	if (LaunchArgs["includeCsharp"] == "true")
	{
		std::cout << "- Including C# project in solution" << std::endl;
		std::string CsGUID = VSProj::WriteCSProj("Games/" + ProjectName + "/Scripts", "CSharpAssembly", "net6.0");

		SLN::Project CSProject;
		CSProject.Name = "CSharpAssembly";
		CSProject.Path = "Scripts";
		CSProject.GUID = CsGUID;
		CSProject.Type = "csproj";
		CppProject.Dependencies.push_back(CsGUID);
		Projects.push_back(CSProject);
	}

	if (LaunchArgs["includeEngine"] == "true")
	{
		std::cout << "- Including engine project in solution" << std::endl;
		SLN::Project EngineProject;
		EngineProject.Name = "Engine";
		EngineProject.Type = "vcxproj";
		EngineProject.Path = "../../EngineSource";
		EngineProject.GUID = "E25491B8-04A8-4B57-9B45-C73718142C84";
		Projects.push_back(EngineProject);

		SLN::Project EngineFolder;
		EngineFolder.GUID = "5B7656A8-A442-4EFA-B737-97B9CA169A78";
		EngineFolder.Name = "Klemmgine";
		EngineFolder.Dependencies = { EngineProject.GUID };
		EngineFolder.Type = "folder";
		Projects.push_back(EngineFolder);
		CppProject.Dependencies.push_back(EngineProject.GUID);
	}
	Projects.push_back(CppProject);
	Projects.push_back(ShaderProject);

	std::cout << "- Writing solution" << std::endl;
	SLN::WriteSolution("Games/" + ProjectName, ProjectName, Projects);
	std::cout << "- Finished writing solution: Games/" << ProjectName << "/" << ProjectName << ".sln" << std::endl;
	
}