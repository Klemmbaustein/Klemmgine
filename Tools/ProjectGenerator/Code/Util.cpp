#include "Util.h"
#include <iostream>

std::string Util::ProjectPath = "";

#if _WIN32
//Include Windows Headers
#include <Windows.h>
#include <Shlobj.h>
#include <shobjidl.h> 
#endif

namespace Util
{
	std::string GetExtension(std::string File)
	{
		return File.substr(File.find_last_of(".") + 1);
	}

	std::string GetFileNameFromPath(std::string FilePath)
	{
		std::string base_filename = FilePath.substr(FilePath.find_last_of("/\\") + 1);
		return base_filename;
	}

	std::vector<std::string> GetAllFilesInFolder(std::string Folder, bool IncludeFolders, bool Recursive, std::string RelativePath)
	{
		if (std::filesystem::is_directory(Folder))
		{
			std::vector<std::string> Files;
			for (const auto& entry : std::filesystem::directory_iterator(Folder))
			{
				if (entry.is_directory() && Recursive && entry.path().filename() != "GENERATED")
				{
					auto f = GetAllFilesInFolder(entry.path().string(),
						IncludeFolders, true, RelativePath + GetFileNameFromPath(entry.path().string()) + "/");
					if (IncludeFolders) Files.push_back(RelativePath + GetFileNameFromPath(entry.path().string()));
					Files.insert(Files.end(), f.begin(), f.end());
				}
				else
				{
					Files.push_back(RelativePath + GetFileNameFromPath(entry.path().string()));
				}
			}
			return Files;
		}
		return std::vector<std::string>();
	}

	void CopyFolderContent(std::string Folder, std::string To, std::set<std::string> FilesToIgnore, std::atomic<float>* Progress, float ProgressAmount)
	{
		try
		{
			size_t NumDirs = 0;
			for (const auto& entry : std::filesystem::directory_iterator(Folder))
			{
				NumDirs++;
			}
			float ProgressFraction = ProgressAmount / NumDirs;

			for (const auto& entry : std::filesystem::directory_iterator(Folder))
			{
				std::string name = entry.path().string();
				name = name.substr(name.find_last_of("/\\") + 1);
				if (!FilesToIgnore.contains(name))
				{
					if (std::filesystem::exists(To + "/" + name))
					{
						std::filesystem::remove_all(To + "/" + name);
					}
					std::filesystem::copy(entry.path(), To + "/" + name, std::filesystem::copy_options::recursive);
				}
				if (Progress)
				{
					*Progress += ProgressFraction;
				}
			}
		}
		catch (std::exception)
		{
			throw "Copy failed";
		}
	}
}