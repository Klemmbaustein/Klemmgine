#include "Assets.h"
#include <Engine/Stats.h>
#include <filesystem>
#include <Engine/Utility/FileUtility.h>

namespace Assets
{
	std::vector<Asset> Assets;

	void ScanForAssets(std::string Path, bool Recursive)
	{
		if (!Recursive)
		{
			Assets.clear();
			if (!(IsInEditor || EngineDebug))
			{
				Path = "Assets/" + Path;
			}
		}
		if (!std::filesystem::exists(Path))
		{
			std::filesystem::create_directories(Path);
		}
		for (const auto& entry : std::filesystem::directory_iterator(Path))
		{
			if (entry.is_directory())
			{
				ScanForAssets(entry.path().string(), true);
			}
			else
			{
				std::string Path = entry.path().string();
#if _WIN32 // Replace all backslashes with forward slashes for consistency.
				for (auto& i : Path)
				{
					if (i == '\\')
					{
						i = '/';
					}
				}
#endif
				Assets.push_back(Asset(Path, FileUtil::GetFileNameFromPath(Path)));
			}
		}
	}


	std::string GetAsset(std::string Name)
	{
		for (const Asset& s : Assets::Assets)
		{
			if (s.Name == Name)
			{
				return s.Filepath;
			}
		}
		return "";
	}
}