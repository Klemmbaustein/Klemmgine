#include <string>
#include <CSharp/CSharpInterop.h>

namespace Project
{
	bool UseNetworkFunctions = false;

	// This is the name of the current project.
	const char* ProjectName = "MyProject";

	// This function returns the map that will be loaded when the game is opened.
	std::string GetStartupScene()
	{
		return CSharp::StaticCall<const char*>(CSharp::LoadCSharpFunction("GetStartupSceneInternally", "Engine", "StringDelegate"));
	}
	
	// This function will be called on startup.
	void OnLaunch()
	{
		CSharp::StaticCall<const char*>(CSharp::LoadCSharpFunction("OnLaunchInternally", "Engine", "VoidDelegate"));
	}
}