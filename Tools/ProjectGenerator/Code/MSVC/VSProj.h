#pragma once
#include <string>

namespace VSProj
{
	// Creates a .vcproj file in the given path with the given name. It will add all files in it's directory to the project.
	// @returns The GUID of the new project.
	std::string WriteVCXProj(std::string Path, std::string Name, std::string WinSdkVer, std::string PlatformToolset);

	// Creates a .csproj file in the given path with the given name. It will add all files in it's directory to the project.
	// @returns The GUID of the new project.
	std::string WriteCSProj(std::string Path, std::string Name, std::string TargetFramework);

}