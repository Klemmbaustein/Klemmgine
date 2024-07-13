#include <string>
#include <Engine/Subsystem/CSharpInterop.h>

namespace Project
{
	bool UseNetworkFunctions = false;

	// This is the name of the current project.
	const char* ProjectName = "Klemmgine";

	// This function returns the map that will be loaded when the game is opened.
	std::string GetStartupScene()
	{
		return CSharpInterop::StaticCall<const char*>(CSharpInterop::CSharpSystem->LoadCSharpFunction("GetStartupSceneInternally", "Engine.Core.Engine", "StringDelegate"));
	}
	
	// This function will be called on startup.
	void OnLaunch()
	{
		CSharpInterop::StaticCall<const char*>(CSharpInterop::CSharpSystem->LoadCSharpFunction("OnLaunchInternally", "Engine.Core.Engine", "VoidDelegate"));
	}
}