#include "Console.h"
#include <Sound/Sound.h>
#include <Engine/EngineProperties.h>
#include <Engine/Importers/Build/Pack.h>
#include <Engine/OS.h>
#include <Rendering/Utility/ShaderManager.h>
#include <set>
#include <map>
#include <sstream>
#include <GL/glew.h>
#include <Engine/Log.h>
#include <Engine/Scene.h>
#include <filesystem>

#include <World/Graphics.h>
#include <World/Stats.h>
#include <World/Assets.h>

namespace Application
{
	void Quit();
}

struct ForceQuitException : std::exception
{
	const char* what() const noexcept
	{
		return "Quit forcefully";
	}
};

namespace Console
{
	std::map<std::string, Variable> ConVars;
	std::map<std::string, Command> Commands;
	std::vector<std::string> ConsoleInput;

	Type::TypeEnum GetDataTypeFromString(std::string& str)
	{
		if (strspn(str.c_str(), "-0123456789") == str.size())
		{
			return Type::E_INT;
		}

		if (strspn(str.c_str(), "-.0123456789") == str.size())
		{
			return Type::E_FLOAT;
		}
		return Type::E_STRING;
	}
	bool IsTypeNumber(Type::TypeEnum GivenType)
	{
		return GivenType == Type::E_INT || GivenType == Type::E_FLOAT;
	}

	template<typename T>
	void SafePtrAssign(void* Ptr, T NewValue)
	{
		T* ptr = (T*)Ptr;
		if (ptr)
		{
			*ptr = NewValue;
		}
	}
	const std::set<char> Seperators
	{
		' ',
		'	',
	};

	const std::set<char> SpecialTokens
	{
		'='
	};
	std::vector<std::string> SeperateToStringArray(const std::string& InString)
	{
		std::stringstream InFile; InFile << InString << " ";
		std::string CurrentWord;
		std::vector<std::string> ProgramStringArray;
		bool InQuotes = false;
		while (!InFile.eof())
		{
			char CurrentChar[2];
			InFile.read(CurrentChar, sizeof(char));
			if (CurrentChar[0] == '\0') break;
			CurrentChar[1] = '\0';
			if (CurrentChar[0] == '"')
			{
				CurrentWord.append(CurrentChar);
				if (!InQuotes)
				{
					InQuotes = true;
					continue;
				}
				else
				{
					InQuotes = false;
					if (!CurrentWord.empty())
					{
						std::string::size_type pos = 0; // Must initialize
						while ((pos = CurrentWord.find("\\n", pos)) != std::string::npos)
						{
							CurrentWord.erase(pos, 2);
							CurrentWord.insert(pos, "\n");
						}
						ProgramStringArray.push_back(CurrentWord);
					}
					CurrentWord.clear();
					continue;
				}
			}
			if (InQuotes)
			{
				CurrentWord.append(CurrentChar);
				continue;
			}

			if (SpecialTokens.contains(CurrentChar[0]))
			{
				if (!CurrentWord.empty()) ProgramStringArray.push_back(CurrentWord);
				ProgramStringArray.push_back(CurrentChar);
				CurrentWord.clear();
				continue;
			}
			else if (Seperators.contains(CurrentChar[0]))
			{
				if (CurrentWord.empty())
				{
					continue;
				}

				ProgramStringArray.push_back(CurrentWord);
				CurrentWord.clear();
				continue;
			}
			else
				CurrentWord.append(CurrentChar);

		}
		return ProgramStringArray;
	}
}

void Console::InitializeConsole()
{
	RegisterCommand(Command("echo",
		[]() {
			Console::ConsoleLog(CommandArgs()[0]);
		}, { Console::Command::Argument("msg", Type::E_STRING) }));


	RegisterCommand(Command("info",
		[]() {
			void (*InfoPrintTypes[3])() =
		{
			[]() {
				ConsoleLog("Version: " + std::string(VERSION_STRING) + (IS_IN_EDITOR ? "-Editor (" : " (") + std::string(ProjectName) + ")");
				ConsoleLog("OS: " + OS::GetOSString());
			},
			[]() {
				ConsoleLog("OpenGL version: " + std::string((const char*)glGetString(GL_VERSION)));
				ConsoleLog("GLSL: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
				ConsoleLog("Window resolution: x=" + std::to_string((int)Graphics::WindowResolution.X) + " y=" + std::to_string((int)Graphics::WindowResolution.Y));
				ConsoleLog("Shadow resolution: " + std::to_string(Graphics::ShadowResolution) + "x" + std::to_string(Graphics::ShadowResolution) + "px");
				ConsoleLog("Shaders: " + std::to_string(GetNumShaders()));
			},
			[]() {
				ConsoleLog("OpenAL version: " + Sound::GetVersionString());
				ConsoleLog("Sounds: " + std::to_string(Sound::GetSounds().size()) + "/255");
			}
		};
	if (!CommandArgs().size())
	{
		ConsoleLog("-----------------------------------[Info]-------------------------------------");
		InfoPrintTypes[0]();
		InfoPrintTypes[1]();
		InfoPrintTypes[2]();
		ConsoleLog("------------------------------------------------------------------------------");
		return;
	}
	if (CommandArgs()[0] == "version")
	{
		ConsoleLog("-----------------------------------[Info]-------------------------------------");
		InfoPrintTypes[0]();
		ConsoleLog("------------------------------------------------------------------------------");
		return;
	}
	if (CommandArgs()[0] == "graphics")
	{
		ConsoleLog("-----------------------------------[Info]-------------------------------------");
		InfoPrintTypes[1]();
		ConsoleLog("------------------------------------------------------------------------------");
		return;
	}
	if (CommandArgs()[0] == "sound")
	{
		ConsoleLog("-----------------------------------[Info]-------------------------------------");
		InfoPrintTypes[2]();
		ConsoleLog("------------------------------------------------------------------------------");
		return;
	}
	ConsoleLog("Unknown info topic: " + CommandArgs()[0], E_ERROR);
	ConsoleLog("Topics are: version, graphics, sound.", E_ERROR);
		}, { Command::Argument("info_type", Type::E_STRING, true) }));


	RegisterCommand(Command("list_pack",
		[]() {
			std::string Pack = CommandArgs()[0];
	auto Result = Pack::GetPackContents(Pack);
	if (Result.size())
	{
		ConsoleLog("Content of .pack file: " + Pack + ": ");
		ConsoleLog("------------------------------------------------------------------------------");
	}
	else
	{
		ConsoleLog("Pack File is emtpy");
		return;
	}
	for (int i = 0; i < Result.size(); i += 2)
	{
		std::string LogString = Result[i].FileName + " (" + std::to_string(Result[i].Content.size()) + " bytes)";
		if (Result.size() > (size_t)i + 1)
		{
			LogString.resize(35, ' ');
			LogString.append(Result[i + 1].FileName + " (" + std::to_string(Result[i + 1].Content.size()) + " bytes)");
		}
		ConsoleLog(LogString);
	}
		}, { Command::Argument("pack_file", Type::E_STRING) }));


	RegisterCommand(Command("find_file",
		[]() {
			std::string File = Assets::GetAsset(CommandArgs()[0]);
	if (File.empty())
	{
		ConsoleLog("Could not find " + CommandArgs()[0], E_ERROR);
		return;
	}
	ConsoleLog(CommandArgs()[0] + " -> " + File);

		}, { Command::Argument("file", Type::E_STRING) }));

	RegisterCommand(Command("loadscene", []() {
		if (std::filesystem::exists(Assets::GetAsset(CommandArgs()[0] + +".jscn")))
		{
			Scene::LoadNewScene(Assets::GetAsset(CommandArgs()[0] + ".jscn"));
		}
		else
		{
			ConsoleLog("Could not find scene \"" + CommandArgs()[0] + "\"", E_ERROR);
		}
		}, { Command::Argument("scene", Type::E_STRING)}));

	RegisterCommand(Command("help", []() {
		std::string CommandString = CommandArgs()[0];
		if (Commands.contains(CommandString))
		{
			auto& FoundCommand = Commands[CommandString];
			ConsoleLog("Help about " + CommandString + ": ");
			PrintArguments(FoundCommand.Arguments);
			return;
		}
		ConsoleLog("Help: " + CommandString + " is not a registered command.", E_ERROR);
		}, { Command::Argument("command", Type::E_STRING) }));

	RegisterCommand(Command("getcommands", []() {
		for (auto i : Commands)
		{
			ConsoleLog(i.first); PrintArguments(i.second.Arguments);
		}
		}, {}));

	RegisterConVar(Variable("vignette", Type::E_FLOAT, &Graphics::Vignette, nullptr));
	RegisterConVar(Variable("vsync", Type::E_BOOL, &Graphics::VSync, nullptr));
}

void Console::ConsoleLog(std::string Message, ConsoleLogType Severity)
{
	Log::Print("[Console]: " + Message, (std::vector<Vector3>{ Log::LogColor::White, Log::LogColor::Yellow, Log::LogColor::Red })[Severity]);
}

bool Console::ExecuteConsoleCommand(std::string Command, bool Verbose)
{
	std::vector<std::string> CommandVec = SeperateToStringArray(Command);
	if (!CommandVec.size()) return false;

	// Check if we are assigning a ConVar by checking if [0] is a ConVar and [1] is '='
	// shadow_resolution = 2048
	//       ^           ^   ^
	//      [0]         [1] [2]
	if (CommandVec.size() >= 2 && ConVars.contains(CommandVec[0]) && CommandVec[1] == "=")
	{
		if (CommandVec.size() == 2)
		{
			ConsoleLog("Expected assignement.", E_ERROR);
			return false;
		}

		if (CommandVec.size() > 3)
		{
			ConsoleLog("Too many arguments. Expected one for assignement.", E_ERROR);
			return false;
		}

		auto& FoundVar = ConVars[CommandVec[0]];
		try
		{
			switch (FoundVar.Type)
			{
			case Type::E_FLOAT:
				SafePtrAssign<float>(FoundVar.Var, std::stof(CommandVec[2]));
				break;
			case Type::E_INT:
				SafePtrAssign<int>(FoundVar.Var, std::stoi(CommandVec[2]));
				break;
			case Type::E_BYTE:
				SafePtrAssign<int>(FoundVar.Var, std::stoi(CommandVec[2]));
				break;
			case Type::E_BOOL:
				SafePtrAssign<bool>(FoundVar.Var, std::stoi(CommandVec[2]));
				break;
			case Type::E_STRING:
				SafePtrAssign<std::string>(FoundVar.Var, CommandVec[2]);
				break;
			default:
				return false;
			}
		}
		catch (std::exception& e)
		{
			ConsoleLog("Invalid type. Expected " + Type::Types[FoundVar.Type], E_ERROR);
			return false;
		}
		ConsoleLog("Assigned " + FoundVar.Name + " = " + CommandVec[2]);
		if (FoundVar.OnSet)
		{
			FoundVar.OnSet();
		}
		return true;
	}


	// Check if the first part of the command is a registered command
	if (Commands.contains(CommandVec[0]))
	{
		for (auto& i : CommandVec)
		{
			if (!ConVars.contains(i))
			{
				continue;
			}
			switch (ConVars[i].Type)
			{
			case Type::E_INT:
				i = std::to_string(*(int*)ConVars[i].Var);
				break;
			case Type::E_BYTE:
				i = std::to_string(*(char*)ConVars[i].Var);
				break;
			case Type::E_FLOAT:
				i = std::to_string(*(int*)ConVars[i].Var);
				break;
			case Type::E_STRING:
				i = *(std::string*)ConVars[i].Var;
				break;
			case Type::E_BOOL:
				i = std::to_string(*(bool*)ConVars[i].Var);
				break;
			default:
				break;
			}
		}

		auto& FoundCommand = Commands[CommandVec[0]];
		ConsoleInput = std::vector<std::string>(CommandVec.begin() + 1, CommandVec.end());

		if (CommandVec.size() - 1 > FoundCommand.Arguments.size())
		{
			ConsoleLog(Command + ": Too many arguments.", E_ERROR);
			PrintArguments(FoundCommand.Arguments, E_ERROR);
			return false;
		}

		for (size_t i = 0; i < FoundCommand.Arguments.size(); i++)
		{
			if (!FoundCommand.Arguments[i].Optional && (int)CommandVec.size() < i + 2)
			{
				ConsoleLog(Command + ": Not enough arguments.", E_ERROR);
				PrintArguments(FoundCommand.Arguments, E_ERROR);
				return false;
			}
			if (IsTypeNumber(FoundCommand.Arguments[i].Type) && !IsTypeNumber(GetDataTypeFromString(CommandVec[i + 1])))
			{
				ConsoleLog("Expected " + Type::Types[FoundCommand.Arguments[i].Type] + " for '" + FoundCommand.Arguments[i].Name + "', found string.", E_ERROR);
				PrintArguments(FoundCommand.Arguments, E_ERROR);
				return false;
			}
		}

		FoundCommand.Function();
		return true;
	}
	return false;
}

Console::Command::Command(std::string Name, void(*Function)(), std::vector<Argument> Arguments)
{
	this->Name = Name;
	this->Function = Function;
	this->Arguments = Arguments;
}

void Console::RegisterConVar(Variable Var)
{
	ConVars.insert(std::pair(Var.Name, Var));
}

void Console::RegisterCommand(Command NewCommand)
{
	bool PreviousOptional = false;
	for (auto& i : NewCommand.Arguments)
	{
		if (i.Optional)
		{
			PreviousOptional = true;
		}
		else if (PreviousOptional)
		{
			ConsoleLog("Error while registering new console command: " + NewCommand.Name + ". All optional arguments should be last.", E_ERROR);
			ConsoleLog("However, " + i.Name + " is not optional but the previous argument was.", E_ERROR);
			return;
		}
	}
	Commands.insert(std::pair(NewCommand.Name, NewCommand));
}

std::vector<std::string> Console::CommandArgs()
{
	return ConsoleInput;
}

void Console::PrintArguments(std::vector<Command::Argument> args, ConsoleLogType Severity)
{
	if (!args.size())
	{
		ConsoleLog("Arguments are: none.", Severity);
	}
	std::string PrintArgs = "Arguments are: ";
	for (auto& i : args)
	{
		PrintArgs.append((i.Optional ? "(optional " : "(") + Type::Types[i.Type] + ") " + i.Name);
		if (i.Name != args[args.size() - 1].Name)
		{
			PrintArgs.append(", ");
		}
	}
	ConsoleLog(PrintArgs, Severity);
}


Console::Command::Argument::Argument(std::string Name, Type::TypeEnum Type, bool Optional)
{
	this->Name = Name;
	this->Type = Type;
	this->Optional = Optional;
}

Console::Variable::Variable(std::string Name, Type::TypeEnum Type, void* Var, void(*OnSet)())
{
	this->Name = Name;
	this->Type = Type;
	this->Var = Var;
	this->OnSet = OnSet;
}
