#pragma once
#include <string>
#include <vector>

namespace Assets
{
	struct Asset
	{
		Asset(std::string Filepath, std::string Name)
		{
			this->Name = Name;
			this->Filepath = Filepath;
		}
		std::string Filepath;
		std::string Name;
	};
	extern std::vector<Asset> Assets;

	void ScanForAssets(std::string Path = "Content/", bool Recursive = false);
	std::string GetAsset(std::string Name);
}