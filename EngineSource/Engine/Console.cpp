#include "Console.h"
#include <CSharp/CSharpInterop.h>
#include <cstring>
#include <Engine/Application.h>
#include <Engine/Build/Pack.h>
#include <Engine/EngineProperties.h>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <Engine/Scene.h>
#include <filesystem>
#include <GL/glew.h>
#include <Rendering/Utility/ShaderManager.h>
#include <set>
#include <Sound/Sound.h>

#include <Engine/File/Assets.h>
#include <Engine/Stats.h>
#include <Rendering/Graphics.h>

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

	NativeType::NativeType GetDataTypeFromString(std::string& str)
	{
		if (strspn(str.c_str(), "-0123456789") == str.size())
		{
			return NativeType::Int;
		}

		if (strspn(str.c_str(), "-.0123456789") == str.size())
		{
			return NativeType::Float;
		}
		return NativeType::String;
	}
	bool IsTypeNumber(NativeType::NativeType GivenType)
	{
		return GivenType == NativeType::Int || GivenType == NativeType::Float;
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
		}, { Console::Command::Argument("msg", NativeType::String) }));

	RegisterCommand(Command("version",
		[]() {
			Console::ConsoleLog(std::string(VERSION_STRING) + (IS_IN_EDITOR ? "-Editor (" : " (") + std::string(Project::ProjectName) + ")");
		}, {}));

	RegisterCommand(Command("info",
		[]() {
			void (*InfoPrintTypes[4])() =
		{
			[]() {
				ConsoleLog("Version: " + std::string(VERSION_STRING) + (IS_IN_EDITOR ? "-Editor (" : " (") + std::string(Project::ProjectName) + ")");
				ConsoleLog("OS: " + OS::GetOSString());
#if SERVER
				ConsoleLog("Running server");
#endif
			},
			[]() {
#if !SERVER
				ConsoleLog("OpenGL version: " + std::string((const char*)glGetString(GL_VERSION)) + ", GLSL: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
				ConsoleLog("Window resolution: x=" + std::to_string((int)Graphics::WindowResolution.X) + " y=" + std::to_string((int)Graphics::WindowResolution.Y));
#else
				ConsoleLog("Build config is server - no graphics");
#endif
			},
			[]() {
#if !SERVER
				ConsoleLog("OpenAL version: " + Sound::GetVersionString());
				ConsoleLog("Sounds: " + std::to_string(Sound::GetSounds().size()) + "/255");
#else
				ConsoleLog("Build config is server - no sound");
#endif
			},
			[]() {
#if ENGINE_CSHARP
				ConsoleLog(CSharp::GetUseCSharp() ? "With C#: Yes" : "With C#: Disabled");
#else
				ConsoleLog("With C#: No");
#endif
			}
		};
	if (!CommandArgs().size())
	{
		ConsoleLog("-----------------------------------[Info]-------------------------------------");
		InfoPrintTypes[0]();
		InfoPrintTypes[1]();
		InfoPrintTypes[2]();
		InfoPrintTypes[3]();
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
	if (CommandArgs()[0] == "csharp")
	{
		ConsoleLog("-----------------------------------[Info]-------------------------------------");
		InfoPrintTypes[3]();
		ConsoleLog("------------------------------------------------------------------------------");
		return;
	}
	ConsoleLog("Unknown info topic: " + CommandArgs()[0], Err);
	ConsoleLog("Topics are: version, graphics, sound.", Err);
		}, { Command::Argument("info_type", NativeType::String, true) }));


	RegisterCommand(Command("listpack",
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
		}, { Command::Argument("pack_file", NativeType::String) }));


	RegisterCommand(Command("find",
		[]() {
			std::string File = Assets::GetAsset(CommandArgs()[0]);
	if (File.empty())
	{
		ConsoleLog("Could not find " + CommandArgs()[0], Err);
		return;
	}
	ConsoleLog(CommandArgs()[0] + " -> " + File);

		}, { Command::Argument("file", NativeType::String) }));

	RegisterCommand(Command("open", []() {
		if (std::filesystem::exists(Assets::GetAsset(CommandArgs()[0] + +".jscn")))
		{
			Scene::LoadNewScene(Assets::GetAsset(CommandArgs()[0] + ".jscn"));
		}
		else
		{
			ConsoleLog("Could not find scene \"" + CommandArgs()[0] + "\"", Err);
		}
		}, { Command::Argument("scene", NativeType::String)}));

	RegisterCommand(Command("help", []() {
		std::string CommandString = CommandArgs()[0];
		if (Commands.contains(CommandString))
		{
			auto& FoundCommand = Commands[CommandString];
			ConsoleLog("Help about " + CommandString + ": ");
			PrintArguments(FoundCommand.Arguments);
			return;
		}
		ConsoleLog("Help: " + CommandString + " is not a registered command.", Err);
		}, { Command::Argument("command", NativeType::String) }));

	RegisterCommand(Command("getcommands", []() {
		for (auto& i : Commands)
		{
			ConsoleLog(i.first); PrintArguments(i.second.Arguments);
		}
		}, {}));

	RegisterCommand(Command("exit", Application::Quit, {}));

	RegisterCommand(Command("stats", []()
		{
			ConsoleLog("FPS: " + std::to_string(1.f / Performance::DeltaTime) + ", Delta: " + std::to_string(Performance::DeltaTime));
			ConsoleLog("Drawcalls: " + std::to_string(Performance::DrawCalls));
		}, {}));

	RegisterCommand(Command("locate", []()
		{
			Application::Timer t;
			std::string FoundFile = Assets::GetAsset(CommandArgs()[0]);
			float Duration = t.Get();
			if (FoundFile.empty())
			{
				FoundFile = "Not found";
			}

			std::string LogMessage = CommandArgs()[0] + " -> " + FoundFile;
			if (CommandArgs().size() > 1 && CommandArgs()[1] != "0")
			{
				LogMessage.append(" (" + std::to_string(Duration) + " seconds)");
			}

			ConsoleLog(LogMessage);
		}, { Command::Argument("file", NativeType::String), Command::Argument("displayFetchTime", NativeType::Bool, true) }));

	RegisterCommand(Command("assetdmp", []()
		{
			for (auto& i : Assets::Assets)
			{
				ConsoleLog(i.Filepath + " - " + i.Name);
			}
		}, {  }));

	RegisterCommand(Command("getclass", []()
		{
			for (const auto& i : Objects::ObjectTypes)
			{
				if (i.Name == CommandArgs()[0])
				{
					ConsoleLog(Objects::GetCategoryFromID(i.ID) + "/" + i.Name);
					return;
				}
			}
			ConsoleLog("Could not find class " + CommandArgs()[0], Warn);
		}, { Command::Argument("objectName", NativeType::String) }));
	RegisterConVar(Variable("wireframe", NativeType::Bool, &Graphics::IsWireframe, nullptr));
	RegisterConVar(Variable("vignette", NativeType::Float, &Graphics::Vignette, nullptr));
	RegisterConVar(Variable("vsync", NativeType::Bool, &Graphics::VSync, nullptr));
	RegisterConVar(Variable("timescale", NativeType::Float, &Performance::TimeMultiplier, nullptr));
	RegisterConVar(Variable("resolution_scale", NativeType::Float, &Graphics::ResolutionScale, []() 
		{
			Graphics::SetWindowResolution(Application::GetWindowSize()); 
		}));
}

void Console::ConsoleLog(std::string Message, ConsoleLogType Severity)
{
	Log::Print("[Console]: " + Message, (std::vector<Vector3>{ Log::LogColor::White, Log::LogColor::Yellow, Log::LogColor::Red })[Severity]);
}

bool Console::ExecuteConsoleCommand(std::string Command)
{
	std::vector<std::string> CommandVec = SeperateToStringArray(Command);
	if (!CommandVec.size())
	{
		return false;
	}

	if (CommandVec.size() == 1 && ConVars.contains(CommandVec[0]))
	{
		std::string ValueString;
		Variable FoundVar = ConVars[CommandVec[0]];
		switch (FoundVar.NativeType)
		{
		case NativeType::Float:
			ValueString = std::to_string(*(float*)FoundVar.Var);
			break;
		case NativeType::Int:
			ValueString = std::to_string(*(int*)FoundVar.Var);
			break;
		case NativeType::Byte:
			ValueString = std::to_string(*(char*)FoundVar.Var);
			break;
		case NativeType::Bool:
			ValueString = std::to_string(*(bool*)FoundVar.Var);
			break;
		case NativeType::String:
			ValueString = *(std::string*)FoundVar.Var;
			break;
		default:
			return false;
		}
		ConsoleLog(FoundVar.Name + " = " + ValueString);
		return true;
	}

	// Check if we are assigning a ConVar by checking if [0] is a ConVar and [1] is '='
	// shadow_resolution = 2048
	//       ^           ^   ^
	//      [0]         [1] [2]
	if (CommandVec.size() >= 2 && ConVars.contains(CommandVec[0]) && CommandVec[1] == "=")
	{
		if (CommandVec.size() == 2)
		{
			ConsoleLog("Expected assignement.", Err);
			return false;
		}

		if (CommandVec.size() > 3)
		{
			ConsoleLog("Too many arguments. Expected one for assignement.", Err);
			return false;
		}

		auto& FoundVar = ConVars[CommandVec[0]];
		try
		{
			switch (FoundVar.NativeType)
			{
			case NativeType::Float:
				SafePtrAssign<float>(FoundVar.Var, std::stof(CommandVec[2]));
				break;
			case NativeType::Int:
				SafePtrAssign<int>(FoundVar.Var, std::stoi(CommandVec[2]));
				break;
			case NativeType::Byte:
				SafePtrAssign<char>(FoundVar.Var, std::stoi(CommandVec[2]));
				break;
			case NativeType::Bool:
				SafePtrAssign<bool>(FoundVar.Var, std::stoi(CommandVec[2]));
				break;
			case NativeType::String:
				SafePtrAssign<std::string>(FoundVar.Var, CommandVec[2]);
				break;
			default:
				return false;
			}
		}
		catch (std::exception& e)
		{
			ConsoleLog("Invalid type. Expected " + NativeType::TypeStrings[FoundVar.NativeType] + " (" + std::string(e.what()) + ")", Err);
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
		// If any part of the command arguments is a convar, we replace it with it's value
		for (auto& i : CommandVec)
		{
			if (!ConVars.contains(i))
			{
				continue;
			}
			switch (ConVars[i].NativeType)
			{
			case NativeType::Int:
				i = std::to_string(*(int*)ConVars[i].Var);
				break;
			case NativeType::Byte:
				i = std::to_string(*(char*)ConVars[i].Var);
				break;
			case NativeType::Float:
				i = std::to_string(*(int*)ConVars[i].Var);
				break;
			case NativeType::String:
				i = *(std::string*)ConVars[i].Var;
				break;
			case NativeType::Bool:
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
			ConsoleLog(Command + ": Too many arguments.", Err);
			PrintArguments(FoundCommand.Arguments, Err);
			return false;
		}

		for (size_t i = 0; i < FoundCommand.Arguments.size(); i++)
		{
			if (!FoundCommand.Arguments[i].Optional && (int)CommandVec.size() < i + 2)
			{
				ConsoleLog(Command + ": Not enough arguments.", Err);
				PrintArguments(FoundCommand.Arguments, Err);
				return false;
			}
			if (FoundCommand.Arguments[i].Optional && CommandVec.size() < i + 2)
			{
				break;
			}

			if (IsTypeNumber(FoundCommand.Arguments[i].NativeType) && !IsTypeNumber(GetDataTypeFromString(CommandVec[i + 1])))
			{
				ConsoleLog("Expected " + NativeType::TypeStrings[FoundCommand.Arguments[i].NativeType] + " for '" + FoundCommand.Arguments[i].Name + "', found string.", Err);
				PrintArguments(FoundCommand.Arguments, Err);
				return false;
			}
		}

		FoundCommand.Function();
		return true;
	}

	if (CommandVec.size())
	{
		ConsoleLog("Unknown convar or command: " + CommandVec[0], Err);
	}
	else
	{
		ConsoleLog("Expected a command.", Err);
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
			ConsoleLog("Error while registering new console command: " + NewCommand.Name + ". All optional arguments should be last.", Err);
			ConsoleLog("However, " + i.Name + " is not optional but the previous argument was.", Err);
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
		return;
	}
	std::string PrintArgs = "Arguments are: ";
	for (auto& i : args)
	{
		PrintArgs.append((i.Optional ? "(optional " : "(") + NativeType::TypeStrings[i.NativeType] + ") " + i.Name);
		if (i.Name != args[args.size() - 1].Name)
		{
			PrintArgs.append(", ");
		}
	}
	ConsoleLog(PrintArgs, Severity);
}


Console::Command::Argument::Argument(std::string Name, NativeType::NativeType NativeType, bool Optional)
{
	this->Name = Name;
	this->NativeType = NativeType;
	this->Optional = Optional;
}

Console::Variable::Variable(std::string Name, NativeType::NativeType NativeType, void* Var, void(*OnSet)())
{
	this->Name = Name;
	this->NativeType = NativeType;
	this->Var = Var;
	this->OnSet = OnSet;
}
