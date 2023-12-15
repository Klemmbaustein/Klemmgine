using Engine.Log;

static class Project
{
	// This function will be called on startup to get the project name.
	public static string GetProjectName()
	{
		return "Untitled";
	}

	// This function will be called on startup. Equivalent to Project.cpp: Project::OnLaunch()
	public static void OnLaunch()
	{
	}

	// This function returns the map that will be loaded when the game is opened. Equivalent to Project.cpp: Project::GetStartupScene()
	public static string GetStartupScene()
	{
		return "";
	}

}