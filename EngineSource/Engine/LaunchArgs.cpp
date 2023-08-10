#include <Engine/Console.h>
#include <vector>
#include <string>
#include <map>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <iostream>
#include <Engine/Scene.h>
#include <World/Graphics.h>
#include <Engine/Application.h>

namespace LaunchArgs
{
	void NeverHideConsole(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -neverhideconsole", Vector3(1, 1, 0));
		OS::SetConsoleCanBeHidden(false);
	}

	void LoadScene(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size() != 1) Log::Print("Unexpected arguments in -loadscene", Vector3(1, 1, 0));
		Scene::LoadNewScene(AdditionalArgs[0]);
		Scene::Tick();
	}

	void NoVSync(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -novsync", Vector3(1, 1, 0));
		Graphics::VSync = false;
	}

	void Wireframe(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -wireframe", Vector3(1, 1, 0));
		Graphics::IsWireframe = true;
	}

	void GetVersion(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -version", Vector3(1, 1, 0));
		Console::ExecuteConsoleCommand("version");
		exit(0);
	}

	void FullScreen(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -fullscreen", Vector3(1, 1, 0));
		Application::SetFullScreen(true);
	}

	std::map<std::string, void(*)(std::vector<std::string>)> Commands =
	{
		std::pair("neverhideconsole", NeverHideConsole),
		std::pair("scene", LoadScene),
		std::pair("novsync", NoVSync),
		std::pair("wireframe", Wireframe),
		std::pair("version", GetVersion),
		std::pair("fullscreen", FullScreen),
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
	}
}