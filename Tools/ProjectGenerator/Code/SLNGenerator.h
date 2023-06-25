#if _WIN32
#pragma once
#include <string>
#include <vector>
#include <map>

namespace sln
{
	struct FilteredItem
	{
		std::string Name;
		std::string Dir;
	};

	void GenerateSolution(std::string path, std::string name, std::map<std::string, std::string> Values, std::vector<std::string> Files, bool WithCSharp, std::string SourceCodeDir = "Code");

	void GenerateVCProj(std::string path, std::string name,
		std::vector<std::string> IncludedFiles, std::vector<std::string> IncludeDirs,
		std::vector<std::string> LibDirs, std::string SourceCodeDir, std::vector<std::string> Shaders, bool WithCSharp);
	void GenerateFilters(std::string path, std::string name, std::vector<FilteredItem> Items, std::vector<std::string> Shaders);
}
#endif