#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <set>

namespace FileUtil
{
	std::string GetFileNameFromPath(std::string FilePath);

	std::string GetFileNameWithoutExtensionFromPath(std::string FilePath);

	std::string GetFilePathWithoutExtension(std::string FilePath);

	std::string wstrtostr(const std::wstring& wstr);

	std::vector<char> StringToCharVector(std::string In);

	std::string VectorToString(std::vector<char> In);

	std::string GetExtension(std::string FileName);

	// Returns a vector containing all files in the folder. If ext != "", only 
	// files with the given extension will be listed.
	std::vector<std::string> GetAllFilesInFolder(std::string Folder, std::string ext = "");

	std::filesystem::file_time_type GetLastWriteTimeOfFolder(std::string Folder, std::set<std::string> FoldersToIgnore);
}