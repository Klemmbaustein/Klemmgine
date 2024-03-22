#pragma once
#include <string>
#include <vector>

namespace ParseFile
{
	struct Object
	{
		std::string Name;
		std::string Path;

		std::vector<std::string> Parents;
		unsigned int Hash = 0;

		bool DerivesFromWorldObject(const std::vector<Object>& AllObjects) const;

		void WriteGeneratedHeader(std::string TargetFolder);
	};

	void WriteToFile(std::string str, std::string File);
	std::vector<Object> ParseFile(std::string Path);
}