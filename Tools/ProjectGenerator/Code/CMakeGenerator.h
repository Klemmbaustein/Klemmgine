#pragma once
#include <string>
#include <vector>
#include <map>

namespace CMake
{
	void WriteCMakesList(std::string path, std::string name, std::map<std::string, std::string> Values, std::vector<std::string> Files, std::string SourceCodeDir = "Code");
}