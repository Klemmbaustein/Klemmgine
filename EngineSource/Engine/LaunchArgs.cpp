#include <Engine/Console.h>
#include <vector>
#include <string>
#include <map>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <iostream>
#include <Engine/Scene.h>
#include <World/Graphics.h>

namespace LaunchArgs
{
	void NeverHideConsole(std::vector<std::string> AdditionalArgs)
	{
		if (AdditionalArgs.size()) Log::Print("Unexpected arguments in -neverhideconsole", Vector3(1, 1, 0));
		OS::SetConsoleCanBeHidden(false);
	}

	void LoadScene(std::vector<std::string> AdditionalArgs)
	{
		Scene::LoadNewScene(AdditionalArgs[0]);
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

	std::map<std::string, void(*)(std::vector<std::string>)> Commands = 
	{
		std::pair("neverhideconsole", NeverHideConsole),
		std::pair("loadscene", LoadScene),
		std::pair("novsync", NoVSync),
		std::pair("wireframe", Wireframe)
	};
	void EvaluateLaunchArguments(std::vector<std::string> Arguments)
	{
		std::vector<std::string> CommandArguments;
		std::string CurrentCommand;
		for (const std::string& arg : Arguments)
		{
			if (arg[0] == '-')
			{
				if (CommandArguments.size())
				{
					if (Commands.contains(CurrentCommand.substr(1)))
					{
						Commands[CurrentCommand.substr(1)](CommandArguments);
					}
					else Log::Print("Unknown Command: " + CurrentCommand[0]);
					CommandArguments.clear();
				}
				CurrentCommand = arg;
			}
			else if(!CurrentCommand.empty())
			{
				CommandArguments.push_back(arg);
			}
		}
		if (Commands.contains(CurrentCommand.substr(1)))
		{
			Commands[CurrentCommand.substr(1)](CommandArguments);
		}
		else Log::Print("Unknown Command: " + CurrentCommand[0]);
	}
}