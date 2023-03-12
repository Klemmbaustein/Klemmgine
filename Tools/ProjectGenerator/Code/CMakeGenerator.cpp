#include "CMakeGenerator.h"
#include "Util.h"
#include "Log.h"
#include <regex>
#include <fstream>

std::string SafeGet(std::string val, std::map<std::string, std::string>& from);

void CMake::WriteCMakesList(std::string path, std::string name, std::map<std::string, std::string> Values, std::vector<std::string> Files, std::string SourceCodeDir)
{
	Log::Print("CMake: Started writing CMakeLists.txt");
	std::vector<std::string> IncludeDirs = { "EngineSource" };
	IncludeDirs.push_back(SafeGet("sdl_include", Values));
	IncludeDirs.push_back(SafeGet("glm_include", Values));
	IncludeDirs.push_back(SafeGet("glew_include", Values));
	IncludeDirs.push_back(SafeGet("assimp_include", Values));
	IncludeDirs.push_back(SafeGet("al_include", Values));
	IncludeDirs.push_back(path + "/Code");
	if (path.empty())
	{
		IncludeDirs.push_back("Tools/BuildTool/Output");
	}
	std::vector<std::string> LibraryDirs = { "Components/lib" };
	LibraryDirs.push_back(SafeGet("sdl_lib", Values));
	LibraryDirs.push_back(SafeGet("glew_lib", Values));
	LibraryDirs.push_back(SafeGet("assimp_lib", Values));
	LibraryDirs.push_back(SafeGet("al_lib", Values));
	std::ifstream in = std::ifstream("Tools/ProjectGenerator/Templates/CMakeLists.txt", std::ios::in);
	std::stringstream buf; buf << in.rdbuf();
	std::string TemplateFile = buf.str();

	std::string LibraryDirectoryString;
	std::string Includes;
	
	std::ofstream FilesList = std::ofstream(path + "ProjectFiles.cmake");

	FilesList << "set(SourceFiles ${SourceFiles}\n";
	for (auto i : Files)
	{
		FilesList << ("	" + SourceCodeDir + i + "\n");
	}
	FilesList << "	# Add your project files here\n)";
	FilesList.close();

	for (auto i : IncludeDirs)
	{
		Includes.append("	" + std::filesystem::absolute(i).string() + "\n");
	}

	std::set<std::string> Libraries =
	{
		"assimp",
		"openal",
		"sdl2",
		"opengl",
		"glew32s",
		"engine_editor"
	};

	std::set<std::string> FoundStaticLibraries;

	for (auto i : LibraryDirs)
	{
		auto files = Util::GetAllFilesInFolder(i, false, false);
		for (auto j : files)
		{
			auto LowerCaseFile = j;
			std::transform(LowerCaseFile.begin(), LowerCaseFile.end(), LowerCaseFile.begin(),
				[](unsigned char c) { return std::tolower(c); });
			for (auto k : Libraries)
			{
				if (LowerCaseFile.find(k) != std::string::npos // SDL2 has "SDL2main.lib and SDL2test.lib and we dont want to include them
					&& LowerCaseFile.find("main") == std::string::npos && LowerCaseFile.find("test") == std::string::npos)
				{
					if (LowerCaseFile.substr(LowerCaseFile.find_last_of(".")) == ".lib")
					{
						FoundStaticLibraries.insert(std::filesystem::absolute(i + j).string());
						Log::Print("found " + k + " in " + std::filesystem::absolute(i + j).string());
					}
				}
			}
		}
	}


	for (auto i : FoundStaticLibraries)
	{
		LibraryDirectoryString.append("	" + i + "\n");
	}
	LibraryDirectoryString.append("opengl32");

	std::regex FilesRegex = std::regex("\\$NAME");
	TemplateFile = std::regex_replace(TemplateFile, FilesRegex, name);
	FilesRegex = std::regex("\\$INCLUDES_LIST");
	TemplateFile = std::regex_replace(TemplateFile, FilesRegex, Includes);
	FilesRegex = std::regex("\\$STATIC_LIBS");
	TemplateFile = std::regex_replace(TemplateFile, FilesRegex, LibraryDirectoryString);
	FilesRegex = std::regex("\\$OUTPUT_PATH");
	TemplateFile = std::regex_replace(TemplateFile, FilesRegex, std::filesystem::absolute(path).string());

	FilesRegex = std::regex("\\\\");
	TemplateFile = std::regex_replace(TemplateFile, FilesRegex, "/");

	std::ofstream Out = std::ofstream(path + "CMakeLists.txt");
	Out << TemplateFile; Out.close();
#if _WIN32
	if (!path.empty())
	{
		system(("cd " + path + " && FindObjects.bat").c_str());
	}
	if (Util::Ask("Done writing CMake files. Compile project? (y/n)", {"y", "n"}) == "y")
	{
		system(("cd " + path + " && cmake -S . -B Build").c_str());
	}
#elif __linux__
	if (!path.empty())
	{
		system(("cd " + path + " && ./FindObjects.sh").c_str());
	}
#endif
}
