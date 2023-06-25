#if _WIN32
#include "SLNGenerator.h"
#include <fstream>
#include <regex>
#include <sstream>
#include "Log.h"
#include <filesystem>
#include "Exception.h"
#include <set>
#include "Util.h"


std::string SafeGet(std::string Value, std::map<std::string, std::string>& Source)
{
	if (Source.contains(Value))
	{
		return Source[Value];
	}
	throw Exception("Value " + Value + " is not defined", "Error while trying to access 'path.txt'");
}

void Replace(std::string& str, std::string from, std::string to)
{
	auto FilesRegex = std::regex(from);
	str = std::regex_replace(str, FilesRegex, to);
}

void sln::GenerateSolution(std::string path, std::string name, std::map<std::string, std::string> Values, std::vector<std::string> Files, bool WithCSharp, std::string SourceCodeDir)
{ 
	Log::Print("sln: Started writing " + name + ".sln");
	std::vector<std::string> IncludeDirs = { "../EngineSource" };
	IncludeDirs.push_back(SafeGet("sdl_include", Values));
	IncludeDirs.push_back(SafeGet("glm_include", Values));
	IncludeDirs.push_back(SafeGet("glew_include", Values));
	IncludeDirs.push_back(SafeGet("assimp_include", Values));
	IncludeDirs.push_back(SafeGet("al_include", Values));
	IncludeDirs.push_back(path + "/Code");
	if (SourceCodeDir == "../EngineSource/")
	{
		IncludeDirs.push_back("../Tools/BuildTool/Output");
	}
	IncludeDirs.push_back(path + "/Code");
	std::vector<std::string> LibraryDirs = { "../Components/lib" };
	LibraryDirs.push_back(SafeGet("sdl_lib", Values));
	LibraryDirs.push_back(SafeGet("glew_lib", Values));
	LibraryDirs.push_back(SafeGet("assimp_lib", Values));
	LibraryDirs.push_back(SafeGet("al_lib", Values));
	if (WithCSharp)
	{
		LibraryDirs.push_back(std::filesystem::absolute("../CSharpCore/lib").string());
	}
	GenerateVCProj(path, name, Files, IncludeDirs, LibraryDirs, SourceCodeDir, Util::GetAllFilesInFolder(path + "Shaders"), WithCSharp);
	std::vector<FilteredItem> FilteredFiles;
	for (auto i : Files)
	{
		if (!std::filesystem::is_directory(path + SourceCodeDir + i.substr(1)))
		{
			FilteredItem NewItem;
			NewItem.Name = SourceCodeDir + i;
			i = "Code" + i;
			auto Slash = i.find_last_of("/\\");
			NewItem.Dir = i.substr(0, Slash);
			FilteredFiles.push_back(NewItem);
		}
		else
		{
			FilteredItem NewItem;
			NewItem.Name = "";
			NewItem.Dir = "Code" + i;
			FilteredFiles.push_back(NewItem);
		}
	}
	GenerateFilters(path, name, FilteredFiles, Util::GetAllFilesInFolder(path + "Shaders"));
	std::ifstream in = std::ifstream("../Tools/ProjectGenerator/Templates/sln", std::ios::in);
	std::stringstream buf; buf << in.rdbuf();
	in.close();
	std::string ProjectFiles;
	std::string TemplateFile = buf.str();

	Replace(TemplateFile, "\\$NAME", name);
	if (WithCSharp && SourceCodeDir != "../EngineSource/")
	{
		Replace(TemplateFile, "\\$CSHARP_PROJ", "Project(\"{9A19103F-16F7-4668-BE54-9A1E7A4F7556}\") = \"CSharpAssembly\", \"Scripts/CSharpAssembly.csproj\", \"{6250B51D-EB42-443E-A7E5-9EA178D109A2}\"\n\
EndProject");
		Replace(TemplateFile, "\\$CSHARP_DEP", "	ProjectSection(ProjectDependencies) = postProject\n\
		{6250B51D-EB42-443E-A7E5-9EA178D109A2} = {6250B51D-EB42-443E-A7E5-9EA178D109A2}\n\
	EndProjectSection");

	}
	else
	{
		Replace(TemplateFile, "\\$CSHARP_PROJ", "");
		Replace(TemplateFile, "\\$CSHARP_DEP", "");
	}
	std::ofstream Out = std::ofstream(path + name + ".sln");
	Out << TemplateFile;
	Out.close();
	if (path != "../EngineSource/")
	{
		system(("cd " + path + " && FindObjects.bat").c_str());
	}
	Log::Print("sln: Done writing " + name + ".sln");
}

void sln::GenerateVCProj(std::string path, std::string name,
	std::vector<std::string> IncludedFiles, std::vector<std::string> IncludeDirs,
	std::vector<std::string> LibDirs, std::string SourceCodeDir, std::vector<std::string> Shaders, bool WithCSharp)
{
	std::ifstream in = std::ifstream("../Tools/ProjectGenerator/Templates/vcxproj", std::ios::in);
	std::stringstream buf; buf << in.rdbuf();
	in.close();
	bool IsStaticLibrary = SourceCodeDir == "../EngineSource/";
	std::string ProjectFiles;
	std::string ProjectHeaders;
	std::string ProjectShaders;
	std::string TemplateFile = buf.str();
	for (auto& i : IncludedFiles)
	{
		if (i.find(".") == std::string::npos)
		{
			continue;
		}
		if (i.substr(i.find_last_of(".")) == ".cpp")
		{
			if (IsStaticLibrary)
			{
				ProjectFiles.append("    <ClCompile Include=\"" + std::filesystem::canonical(SourceCodeDir + i).string() + "\"/>\n");
			}
			else
			{
				ProjectFiles.append("    <ClCompile Include=\"" + SourceCodeDir + i + "\"/>\n");
			}
		}
		else if (i.substr(i.find_last_of(".")) == ".h")
		{
			if (IsStaticLibrary)
			{
				ProjectHeaders.append("    <ClInclude Include=\"" + std::filesystem::canonical(SourceCodeDir + i).string() + "\"/>\n");
			}
			else
			{
				ProjectHeaders.append("    <ClInclude Include=\"" + SourceCodeDir + i + "\"/>\n");
			}
		}
	}

	for (auto& i : Shaders)
	{
		ProjectShaders.append("<None Include = \"" + std::filesystem::absolute(path + "Shaders" + i).string() + "\" />\n");
	}

	Replace(TemplateFile, "\\$FILES", ProjectFiles);
	Replace(TemplateFile, "\\$INCLUDES", ProjectHeaders);
	Replace(TemplateFile, "\\$NAME", name);
	Replace(TemplateFile, "\\$TYPE", IsStaticLibrary ? "StaticLibrary" : "Application");
	Replace(TemplateFile, "\\$SHADERS", ProjectShaders);
	std::string IncludeDirsString;
	for (auto& i : IncludeDirs)
	{
		IncludeDirsString.append(std::filesystem::absolute(i).string() + ";");
	}
	Replace(TemplateFile, "\\$INCLUDE_DIRS", IncludeDirsString);
	IncludeDirsString.clear();
	for (auto& i : LibDirs)
	{
		IncludeDirsString.append(std::filesystem::absolute(i).string() + ";");
	}

	std::string  CSharpLib = WithCSharp ? "nethost.lib;" : "";

	Replace(TemplateFile, "\\$CSHARP_PPDEF", WithCSharp ? "ENGINE_CSHARP;" : "");
	Replace(TemplateFile, "\\$LIBRARY_DIRS", IsStaticLibrary ? "" : IncludeDirsString);
	Replace(TemplateFile, "\\$PREBUILDEVENT", IsStaticLibrary ? "" : "<PreBuildEvent>\n\
	<Command>FindObjects.bat</Command>\n\
</PreBuildEvent>");

	if (!IsStaticLibrary)
	{
		Replace(TemplateFile, "\\$LIBRARIES_RELEASE",
			"assimp.lib;OpenAL32.lib;SDL2.lib;opengl32.lib;glew32s.lib;Engine_Release.lib;" + CSharpLib);
		Replace(TemplateFile, "\\$LIBRARIES_DEBUG",
			"assimp.lib;OpenAL32.lib;SDL2.lib;opengl32.lib;glew32s.lib;Engine_Debug.lib;" + CSharpLib);
		Replace(TemplateFile, "\\$LIBRARIES_EDITOR",
			"assimp.lib;OpenAL32.lib;SDL2.lib;opengl32.lib;glew32s.lib;Engine_Editor.lib;" + CSharpLib);
	}
	std::ofstream Out = std::ofstream(path + name + ".vcxproj");
	Out << TemplateFile;
	Out.close();
	Log::Print("sln: " + name + ".vcxproj successfully written");
}

void sln::GenerateFilters(std::string path, std::string name, std::vector<FilteredItem> Items, std::vector<std::string> Shaders)
{
	std::ifstream in = std::ifstream("../Tools/ProjectGenerator/Templates/vcxproj.filters", std::ios::in);
	std::stringstream buf; buf << in.rdbuf();
	in.close();
	std::string ProjectFiles;
	std::string ProjectHeaders;
	std::string ProjectShaders;
	std::string FiltersString;
	std::string TemplateFile = buf.str();
	std::set<std::string> Filters;
	Filters.insert("Code");
	Filters.insert("Code\\Objects");
	Filters.insert("Code\\UI");



	for (auto& i : Items)
	{
		Replace(i.Dir, "/", "\\");
		Filters.insert(i.Dir);
		if (i.Name.find(".") == std::string::npos)
		{
			continue;
		}
		if (path == "../")
		{
			if (i.Name.substr(i.Name.find_last_of(".")) == ".cpp")
			{
				ProjectFiles.append("	<ClCompile Include=\"" + std::filesystem::absolute(i.Name).string() + "\">\n\
		<Filter>" + i.Dir + "</Filter>\n\
	</ClCompile>\n");
			}
			else if (i.Name.substr(i.Name.find_last_of(".")) == ".h")
			{
				ProjectHeaders.append("	<ClInclude Include=\"" + std::filesystem::absolute(i.Name).string() + "\">\n\
		<Filter>" + i.Dir + "</Filter>\n\
	</ClInclude>\n");
			}
		}
		else
		{
			if (i.Name.substr(i.Name.find_last_of(".")) == ".cpp")
			{
				ProjectFiles.append("	<ClCompile Include=\"" + i.Name + "\">\n\
		<Filter>" + i.Dir + "</Filter>\n\
	</ClCompile>\n");
			}
			else if (i.Name.substr(i.Name.find_last_of(".")) == ".h")
			{
				ProjectHeaders.append("	<ClInclude Include=\"" + i.Name + "\">\n\
		<Filter>" + i.Dir + "</Filter>\n\
	</ClInclude>\n");
			}
		}
	}

	if (std::filesystem::exists(path + "/Shaders"))
	{
		Filters.insert("Shaders");
		for (auto& i : std::filesystem::directory_iterator(path + "\\Shaders"))
		{
			if (std::filesystem::is_directory(i))
			{
				Filters.insert("Shaders\\" + i.path().filename().string());
			}
		}
		for (auto& i : Shaders)
		{
			std::string Dir = "Shaders" + i;
			Dir = Dir.substr(0, Dir.find_last_of("/\\"));

			Replace(Dir, "/", "\\");

			ProjectShaders.append("    <None Include=\"" + std::filesystem::absolute(path + "Shaders" + i).string() + "\">\n\
		<Filter>" + Dir + "</Filter>\n\
	</None>\n");
		}
	}

	for (auto& i : Filters)
	{
		FiltersString.append("    <Filter Include=\"" + i + "\">\n	</Filter>\n");
	}

	Replace(TemplateFile, "\\$FILES", ProjectFiles);
	Replace(TemplateFile, "\\$FILTERS", FiltersString);
	Replace(TemplateFile, "\\$INCLUDES", ProjectHeaders);
	Replace(TemplateFile, "\\$SHADERS", ProjectShaders);
	std::ofstream Out = std::ofstream(path + name + ".vcxproj.filters");
	Out << TemplateFile;
	Out.close();
}
#endif