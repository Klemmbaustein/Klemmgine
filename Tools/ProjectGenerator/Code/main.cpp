#include "MSVC/VSProj.h"
#include "MSVC/SLN.h"
#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>

std::vector<std::string> DependencyDlls =
{
	"bDependencies/assimp/bin/Release/assimp-vc143-mt.dll",
	"bDependencies/openal-soft/Release/OpenAL32.dll",
	"aDependencies/SDL/VisualC/SDL/x64/Release/SDL2.dll",
	"aDependencies/SDL_net/Build/Release/SDL2_net.dll",
	"aCSharpCore/lib/nethost.dll"
};

int main(int argc, char** argv)
{
	std::string ExecPath = argv[0];
	ExecPath = ExecPath.substr(0, ExecPath.find_last_of("\\/"));
	std::filesystem::current_path(ExecPath);

	std::cout << "Klemmgine Project Generator v1.0";
#if ENGINE_NO_SOURCE
	std::cout << " - without engine source";
#endif
	std::cout << std::endl;

	std::map<std::string, std::string> LaunchArgs =
	{
		std::pair("winSdk", "10.0"),
		std::pair("toolset", "v143"),
		std::pair("projectName", ""),
		std::pair("includeEngine", "true"),
		std::pair("includeCsharp", "true"),
		std::pair("upgrade", "false"),
		std::pair("onlyBuildFiles", "false"),
		std::pair("ciBuild", "false"),
		std::pair("projectPath", "")
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

	if (LaunchArgs["projectPath"].empty())
	{
		LaunchArgs["projectPath"] = "Games/" + ProjectName;
	}

	if (LaunchArgs["onlyBuildFiles"] == "false")
	{
		if (std::filesystem::exists(LaunchArgs["projectPath"]) && LaunchArgs["upgrade"] == "false")
		{
			std::cout << "Warning: " << LaunchArgs["projectPath"] << " already exists. Replacing..." << std::endl;
			std::filesystem::remove_all(LaunchArgs["projectPath"]);
		}
		else
		{
			if (std::filesystem::exists(LaunchArgs["projectPath"] + "/.vs"))
			{
				std::filesystem::remove_all(LaunchArgs["projectPath"] + "/.vs");
				std::cout << "Upgrading project files of " << ProjectName << std::endl;
			}
		}

		std::cout << "Copying project files" << std::endl;
		std::filesystem::create_directories(LaunchArgs["projectPath"] + "/Code/Objects");
		std::filesystem::copy("Tools/ProjectGenerator/ProjectFiles", LaunchArgs["projectPath"],
			std::filesystem::copy_options::recursive
			| std::filesystem::copy_options::overwrite_existing);
#if ENGINE_NO_SOURCE
		std::filesystem::copy("Tools/ProjectGenerator/ProjectFilesNoSource", LaunchArgs["projectPath"],
			std::filesystem::copy_options::recursive
			| std::filesystem::copy_options::overwrite_existing);
#endif

#if !ENGINE_NO_SOURCE
		for (auto& i : DependencyDlls)
		{
			std::string Name = i.substr(1);
			std::string Path;
			if (i[0] == 'b')
			{
				Path = "/bin";
			}

			if (!std::filesystem::exists(Name))
			{
				std::cout << "Could not find " << Name << ". Ensure you have the project setup correctly." << std::endl;
				exit(1);
			}
			std::filesystem::create_directories(LaunchArgs["projectPath"] + Path);
			std::filesystem::copy(Name, LaunchArgs["projectPath"] + Path, std::filesystem::copy_options::overwrite_existing);
			std::cout << (LaunchArgs["projectPath"] + Path) << std::endl;;
		}
#endif
	}

	std::string CppGUID = VSProj::WriteVCXProj(LaunchArgs["projectPath"] + "/Code", ProjectName, "10.0", "v143", true);

	std::cout << "Generating solution..." << std::endl;

	std::vector<SLN::Project> Projects;

#if ENGINE_NO_SOURCE
	if (LaunchArgs["ciBuild"] != "false")
#endif
	{
		SLN::Project CppProject;
		CppProject.Name = ProjectName;
		CppProject.Path = "Code";
		CppProject.GUID = "A2BEFDE1-9019-4A47-839E-545ACCF559F2";
		CppProject.Type = "vcxproj";
		Projects.push_back(CppProject);
	}
	if (LaunchArgs["includeCsharp"] == "true")
	{
		std::cout << "- Including C# project in solution" << std::endl;
		std::string CsGUID = VSProj::WriteCSProj(LaunchArgs["projectPath"] + "/Scripts", "CSharpAssembly", "net7.0");

		SLN::Project CSProject;
		CSProject.Name = "CSharpAssembly";
		CSProject.Path = "Scripts";
		CSProject.GUID = CsGUID;
		CSProject.Type = "csproj";
#if !ENGINE_NO_SOURCE
		Projects[0].Dependencies.push_back(CsGUID);
#endif
		Projects.push_back(CSProject);
	}

#if !ENGINE_NO_SOURCE
	if (LaunchArgs["includeEngine"] == "true")
	{
		std::cout << "- Including engine project in solution" << std::endl;
		SLN::Project EngineProject;
		EngineProject.Name = "Engine";
		EngineProject.Type = "vcxproj";
		EngineProject.Path = std::filesystem::current_path().append("EngineSource").string();
		EngineProject.GUID = "E25491B8-04A8-4B57-9B45-C73718142C84";
		Projects.push_back(EngineProject);

		SLN::Project EngineFolder;
		EngineFolder.GUID = "5B7656A8-A442-4EFA-B737-97B9CA169A78";
		EngineFolder.Name = "Klemmgine";
		EngineFolder.Dependencies = { EngineProject.GUID };
		EngineFolder.Type = "folder";
		Projects.push_back(EngineFolder);
		Projects[0].Dependencies.push_back(EngineProject.GUID);
	}
#endif

	std::string ShaderGUID = VSProj::WriteVCXProj(LaunchArgs["projectPath"] + "/Shaders", "Shaders", "10.0", "v143", false);
#if 1
	SLN::Project ShaderProject;
	ShaderProject.Name = "Shaders";
	ShaderProject.Path = "Shaders";
	ShaderProject.GUID = "91315EA7-5B25-4D8E-A732-C66EB73E510E";
	ShaderProject.Type = "vcxproj";
	Projects.push_back(ShaderProject);
#endif

	if (LaunchArgs["onlyBuildFiles"] == "false")
	{
		std::cout << "- Writing solution" << std::endl;
		SLN::WriteSolution(LaunchArgs["projectPath"], ProjectName, Projects);
		std::cout << "- Finished writing solution: " << LaunchArgs["projectPath"] << "/" << ProjectName << ".sln" << std::endl;
	}
}