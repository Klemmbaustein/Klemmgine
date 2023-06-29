#if !RELEASE
#pragma once
#include <string>
#include <vector>

namespace Build
{
	std::string TryBuildProject(std::string TargetFolder);

	// Builds the current .sln file in the project. It returns the return value of the command.
	// 0 means success, 1 means failure.
	int BuildCurrentSolution(std::string Configuration);

	std::string GetProjectBuildName();
}
#endif