#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <set>

/**
* @file
* 
* @brief
* Utility functions for files.
*/

/**
* Namespace containing utility functions for files.
*/
namespace FileUtil
{
	std::string GetFileNameFromPath(std::string FilePath);

	std::string GetPath(std::string FileName);

	/**
	* @brief
	* It is a scientific fact that this function has the best name.
	* 
	* Returns the given FilePath, without the path or extension.
	* Example: `Content/Meshes/Cube.jsm` -> `Cube`
	* 
	* @param FilePath
	* The path to the file
	* 
	* @return
	* The given FilePath, without the path or extension.
	*/
	std::string GetFileNameWithoutExtensionFromPath(std::string FilePath);

	std::string GetFilePathWithoutExtension(std::string FilePath);

	std::string wstrtostr(const std::wstring& wstr);

	std::vector<char> StringToCharVector(std::string In);

	/**
	* @brief
	* Gets the extension from a file name.
	*/
	std::string GetExtension(std::string FileName);

	/**
	* @brief
	* Gets the content of a file, as a string.
	* 
	* @param FilePath
	* The file to load
	* 
	* @return
	* The content of the file, as a string.
	*/
	std::string GetFileContent(std::string FilePath);

	// Returns a vector containing all files in the folder. If ext != "", only 
	// files with the given extension will be listed.
	std::vector<std::string> GetAllFilesInFolder(std::string Folder, std::string ext = "");

	std::filesystem::file_time_type GetLastWriteTimeOfFolder(std::string Folder, std::set<std::string> FoldersToIgnore);
}