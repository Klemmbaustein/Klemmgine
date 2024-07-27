#if !RELEASE
#pragma once
#include <string>
#include <vector>

namespace Build
{
	std::string TryBuildProject(std::string TargetFolder);

	/**
	* @brief
	* Builds the current.sln file of the project.
	* 
	* @return
	* The return value of the command.
	* 0 means success, 1 means failure.
	* 
	* @ingroup Editor
	*/
	int BuildCurrentSolution(std::string Configuration);

	std::string GetProjectBuildName();

	std::string GetSolutionName();

	namespace CMake
	{
		void SetCompilationProcessorCount(uint32_t Count);

		std::string GetBuildRootPath(std::string Configuration);
		bool BuildWithConfig(std::string Configuration, std::string Args = "");

		bool IsUsingCMake();

		void SetMSBuildConfig(std::string Name);
		std::string GetMSBuildConfig();
	}
}
#endif