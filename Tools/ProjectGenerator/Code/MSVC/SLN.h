#pragma once
#include <vector>
#include <string>
#include <map>

namespace SLN
{
	struct Project
	{
		std::string GUID;
		std::string Path;
		std::string Name;
		std::string Type;
		std::vector<std::string> Dependencies;
	};

	const inline std::map<std::string, std::string> GUIDTypes =
	{
		std::pair("vcxproj", "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942"),
		std::pair("csproj", "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC"),
		std::pair("folder", "2150E333-8FDC-42A3-9474-1A3956D46DE8"),
	};

	std::string GetGUID();

	void WriteSolution(std::string Path, std::string Name, std::vector<Project> Projects);
}