#pragma once
#include <string>

namespace Cubemap
{
	unsigned int LoadCubemapFile(std::string File);
	void UnloadCubemapFile(unsigned int map);
}