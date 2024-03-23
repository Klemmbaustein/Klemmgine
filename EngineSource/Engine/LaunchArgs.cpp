#include <Engine/Subsystem/Console.h>
#include <vector>
#include <string>
#include <map>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <iostream>
#include <Engine/Subsystem/Scene.h>
#include <Rendering/Graphics.h>
#include <Engine/Application.h>

namespace LaunchArgs
{
	static void NeverHideConsole(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size())
		{
			Log::Print("Unexpected arguments in -neverhideconsole", Log::LogColor::Yellow);
		}
		OS::SetConsoleCanBeHidden(false);
	}

	static void LoadScene(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size() != 1)
		{
			Log::Print("Unexpected or missing arguments in -loadscene", Log::LogColor::Yellow);
			return;
		}
		Application::StartupSceneOverride = AdditionalArgs[0];
	}

	static void NoVSync(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size())
		{
			Log::Print("Unexpected arguments in -novsync", Log::LogColor::Yellow);
		}
		Graphics::VSync = false;
	}

	static void Wireframe(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -wireframe", Log::LogColor::Yellow);
		Graphics::IsWireframe = true;
	}

	static void GetVersion(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -version", Log::LogColor::Yellow);
		Console::ExecuteConsoleCommand("version");
		exit(0);
	}

	static void FullScreen(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -fullscreen", Log::LogColor::Yellow);
		Application::SetFullScreen(true);
	}

	static void NoStartupInfo(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -nostartupinfo", Log::LogColor::Yellow);
		Application::ShowStartupInfo = false;
	}

	static void Connect(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size() != 1)
		{
			Log::Print("Unexpected or missing arguments in -connect", Log::LogColor::Yellow);
			return;
		}
		Console::ExecuteConsoleCommand("connect " + AdditionalArgs[0]);
	}

	static void EditorPath(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size() != 1)
		{
			Log::Print("Unexpected or missing arguments in -editorPath", Log::LogColor::Yellow);
			return;
		}
		Application::SetEditorPath(AdditionalArgs[0]);
	}
	std::map<std::string, void(*)(std::vector<std::string>)> Commands =
	{
		std::pair("neverhideconsole", &NeverHideConsole),
		std::pair("scene", &LoadScene),
		std::pair("novsync", &NoVSync),
		std::pair("wireframe", &Wireframe),
		std::pair("version", &GetVersion),
		std::pair("fullscreen", &FullScreen),
		std::pair("nostartupinfo", &NoStartupInfo),
		std::pair("connect", &Connect),
#if !RELEASE
		std::pair("editorPath", &EditorPath),
#endif
#if SERVER
		std::pair("quitondisconnect", [](std::vector<std::string> arg) {
			Console::ExecuteConsoleCommand("quitondisconnect");
		}),
#endif
	};
	void EvaluateLaunchArguments(std::vector<std::string> Arguments)
	{
		std::vector<std::string> CommandArguments;
		std::string CurrentCommand;
		for (const std::string& arg : Arguments)
		{
			if (arg[0] == '-')
			{
				if (!CurrentCommand.empty() && Commands.contains(CurrentCommand.substr(1)))
				{
					Commands[CurrentCommand.substr(1)](CommandArguments);
				}
				else if (!CurrentCommand.empty())
				{
					Log::Print("Unknown launch argument: " + CurrentCommand);
				}
				CommandArguments.clear();
				CurrentCommand = arg;
			}
			else if(!CurrentCommand.empty())
			{
				CommandArguments.push_back(arg);
			}
		}
		if (!CurrentCommand.empty() && Commands.contains(CurrentCommand.substr(1)))
		{
			Commands[CurrentCommand.substr(1)](CommandArguments);
		}
		else if (!CurrentCommand.empty())
		{
			Log::Print("Unknown launch argument: " + CurrentCommand);
		}
	}
}