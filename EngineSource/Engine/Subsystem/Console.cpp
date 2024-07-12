#include "Console.h"
#include <Engine/Subsystem/CSharpInterop.h>
#include <cstring>
#include <Engine/Application.h>
#include <Engine/Build/Pack.h>
#include <Engine/EngineProperties.h>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <Engine/Subsystem/Scene.h>
#include <filesystem>
#include <GL/glew.h>
#include <Engine/Subsystem/Sound.h>

#include <Engine/File/Assets.h>
#include <Engine/Stats.h>
#include <Rendering/Graphics.h>
#include <iostream>
#if __linux__
#include <poll.h>
#endif

Console* Console::ConsoleSystem = nullptr;
#if _WIN32
std::condition_variable Console::ConsoleConditionVariable;
std::mutex Console::ConsoleReadMutex;
std::deque<std::string> Console::ConsoleLines;
#endif

std::vector<std::string> Console::SeparateToStringArray(const std::string& InString)
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

		if (ConsoleSystem->SpecialTokens.contains(CurrentChar[0]))
		{
			if (!CurrentWord.empty()) ProgramStringArray.push_back(CurrentWord);
			ProgramStringArray.push_back(CurrentChar);
			CurrentWord.clear();
			continue;
		}
		else if (ConsoleSystem->Separators.contains(CurrentChar[0]))
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

std::string Console::GetVariableValueString(Variable Var)
{
	switch (Var.NativeType)
	{
	case NativeType::Int:
		return std::to_string(*(int*)Var.Var);
	case NativeType::Byte:
		return std::to_string(*(char*)Var.Var);
	case NativeType::Float:
		return std::to_string(*(float*)Var.Var);
	case NativeType::String:
		return *(std::string*)Var.Var;
	case NativeType::Bool:
		return std::to_string(*(bool*)Var.Var);
	default:
		return "";
	}
}

Console::Console()
{
	ConsoleSystem = this;
	Name = "Console";
	RegisterCommand(Command("echo",
		[]() {
			ConsoleSystem->Print(ConsoleSystem->CommandArgs()[0]);
		}, { Console::Command::Argument("msg", NativeType::String) }));

	RegisterCommand(Command("version",
		[]() {
			ConsoleSystem->Print(std::string(VERSION_STRING) + (IS_IN_EDITOR ? "-Editor (" : " (") + std::string(Project::ProjectName) + ")");
		}, {}));

	RegisterCommand(Command("info",
		[]() {
			void (*InfoPrintTypes[4])() =
			{
				[]() {
					ConsoleSystem->Print("Version: " + std::string(VERSION_STRING) + (IS_IN_EDITOR ? "-Editor (" : " (") + std::string(Project::ProjectName) + ")");
					ConsoleSystem->Print("OS: " + OS::GetOSString());
#if SERVER
					ConsoleSystem->Print("Running server");
#endif
				},
				[]() {
#if !SERVER
					ConsoleSystem->Print("OpenGL version: " + std::string((const char*)glGetString(GL_VERSION)) + ", GLSL: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
					ConsoleSystem->Print("Window resolution: x=" + std::to_string((int)Graphics::WindowResolution.X) + " y=" + std::to_string((int)Graphics::WindowResolution.Y));
#else
					ConsoleSystem->Print("Build config is server - no graphics");
#endif
				},
				[]() {
#if !SERVER
					ConsoleSystem->Print("OpenAL version: " + Sound::SoundSystem->GetVersionString());
					ConsoleSystem->Print("Sounds: " + std::to_string(Sound::SoundSystem->GetSounds().size()) + "/255");
#else
					ConsoleSystem->Print("Build config is server - no sound");
#endif
				},
				[]() {
#if ENGINE_CSHARP
					ConsoleSystem->Print(CSharpInterop::GetUseCSharp() ? "With C#: Yes" : "With C#: Disabled");
#else
					ConsoleSystem->Print("With C#: No");
#endif
				}
			};
			if (!ConsoleSystem->CommandArgs().size())
			{
				ConsoleSystem->Print("-----------------------------------[Info]-------------------------------------");
				InfoPrintTypes[0]();
				InfoPrintTypes[1]();
				InfoPrintTypes[2]();
				InfoPrintTypes[3]();
				ConsoleSystem->Print("------------------------------------------------------------------------------");
				return;
			}
			if (ConsoleSystem->CommandArgs()[0] == "version")
			{
				ConsoleSystem->Print("-----------------------------------[Info]-------------------------------------");
				InfoPrintTypes[0]();
				ConsoleSystem->Print("------------------------------------------------------------------------------");
				return;
			}
			if (ConsoleSystem->CommandArgs()[0] == "graphics")
			{
				ConsoleSystem->Print("-----------------------------------[Info]-------------------------------------");
				InfoPrintTypes[1]();
				ConsoleSystem->Print("------------------------------------------------------------------------------");
				return;
			}
			if (ConsoleSystem->CommandArgs()[0] == "sound")
			{
				ConsoleSystem->Print("-----------------------------------[Info]-------------------------------------");
				InfoPrintTypes[2]();
				ConsoleSystem->Print("------------------------------------------------------------------------------");
				return;
			}
			if (ConsoleSystem->CommandArgs()[0] == "csharp")
			{
				ConsoleSystem->Print("-----------------------------------[Info]-------------------------------------");
				InfoPrintTypes[3]();
				ConsoleSystem->Print("------------------------------------------------------------------------------");
				return;
			}
			ConsoleSystem->Print("Unknown info topic: " + ConsoleSystem->CommandArgs()[0], ErrorLevel::Error);
			ConsoleSystem->Print("Topics are: version, graphics, sound.", ErrorLevel::Error);
		}, { Command::Argument("info_type", NativeType::String, true) }));


	RegisterCommand(Command("list_pack",
		[]() {
			std::string Pack = ConsoleSystem->CommandArgs()[0];
			auto Result = Pack::GetPackContents(Pack);
			if (Result.size())
			{
				ConsoleSystem->Print("Content of .pack file: " + Pack + ": ");
				ConsoleSystem->Print("------------------------------------------------------------------------------");
			}
			else
			{
				ConsoleSystem->Print("Pack File is emtpy");
				return;
			}
			for (size_t i = 0; i < Result.size(); i += 2)
			{
				std::string LogString = Result[i].FileName + " (" + std::to_string(Result[i].Content.size()) + " bytes)";
				if (Result.size() > (size_t)i + 1)
				{
					LogString.resize(35, ' ');
					LogString.append(Result[i + 1].FileName + " (" + std::to_string(Result[i + 1].Content.size()) + " bytes)");
				}
				ConsoleSystem->Print(LogString);
			}
		}, { Command::Argument("pack_file", NativeType::String) }));


	RegisterCommand(Command("open", []() {
		if (std::filesystem::exists(Assets::GetAsset(ConsoleSystem->CommandArgs()[0] + +".jscn")))
		{
			Scene::LoadNewScene(Assets::GetAsset(ConsoleSystem->CommandArgs()[0] + ".jscn"));
		}
		else
		{
			ConsoleSystem->Print("Could not find scene \"" + ConsoleSystem->CommandArgs()[0] + "\"", ErrorLevel::Error);
		}
		}, { Command::Argument("scene", NativeType::String) }));

	RegisterCommand(Command("help", []() {
		std::string CommandString = ConsoleSystem->CommandArgs()[0];
		if (ConsoleSystem->Commands.contains(CommandString))
		{
			auto& FoundCommand = ConsoleSystem->Commands[CommandString];
			ConsoleSystem->Print("Help about " + CommandString + ": ");
			ConsoleSystem->PrintArguments(FoundCommand.Arguments);
			return;
		}
		ConsoleSystem->Print("Help: " + CommandString + " is not a registered command.", ErrorLevel::Error);
		}, { Command::Argument("command", NativeType::String) }));

	RegisterCommand(Command("get_commands", []() {
		for (auto& i : ConsoleSystem->Commands)
		{
			ConsoleSystem->Print(i.first);
			ConsoleSystem->PrintArguments(i.second.Arguments);
		}
		}, {}));

	RegisterCommand(Command("exit", Application::Quit, {}));

	RegisterCommand(Command("stats", []()
		{
			ConsoleSystem->Print("FPS: " + std::to_string(1.f / Stats::DeltaTime) + ", Delta: " + std::to_string(Stats::DeltaTime));
			ConsoleSystem->Print("DrawCalls: " + std::to_string(Stats::DrawCalls));
		}, {}));

	RegisterCommand(Command("locate", []()
		{
			Application::Timer t;
			std::string FoundFile = Assets::GetAsset(ConsoleSystem->CommandArgs()[0]);
			float Duration = t.Get();
			if (FoundFile.empty())
			{
				FoundFile = "Not found";
			}

			std::string LogMessage = ConsoleSystem->CommandArgs()[0] + " -> " + FoundFile;
			if (ConsoleSystem->CommandArgs().size() > 1 && ConsoleSystem->CommandArgs()[1] != "0")
			{
				LogMessage.append(" (" + std::to_string(Duration) + " seconds)");
			}

			ConsoleSystem->Print(LogMessage);
		}, { Command::Argument("file", NativeType::String), Command::Argument("print_search_time", NativeType::Bool, true) }));

	RegisterCommand(Command("asset_dump", []()
		{
			for (auto& i : Assets::Assets)
			{
				ConsoleSystem->Print(i.Filepath + " - " + i.Name);
			}
		}, {  }));

	RegisterCommand(Command("get_class", []()
		{
			for (const auto& i : Objects::ObjectTypes)
			{
				if (i.Name == ConsoleSystem->CommandArgs()[0])
				{
					ConsoleSystem->Print(Objects::GetCategoryFromID(i.ID) + "/" + i.Name);
					return;
				}
			}
			ConsoleSystem->Print("Could not find class " + ConsoleSystem->CommandArgs()[0], ErrorLevel::Error);
		}, { Command::Argument("objectName", NativeType::String) }));
	RegisterConVar(Variable("wireframe", NativeType::Bool, &Graphics::IsWireframe, nullptr));
	RegisterConVar(Variable("vignette", NativeType::Float, &Graphics::Vignette, nullptr));
	RegisterConVar(Variable("vsync", NativeType::Bool, &Graphics::VSync, nullptr));
	RegisterConVar(Variable("timescale", NativeType::Float, &Stats::TimeMultiplier, nullptr));
	RegisterConVar(Variable("resolution_scale", NativeType::Float, &Graphics::ResolutionScale, []()
		{
			Graphics::SetWindowResolution(Application::GetWindowSize());
		}));

	RegisterCommand(Command("crash", []()
		{
			abort();
		}, {  }));

#if _WIN32
	ReadConsoleThread = std::thread(ReadConsole);
#endif
}

Console::~Console()
{
#if _WIN32
	ReadConsoleThread.detach();
#endif
	ConVars.clear();
	Commands.clear();
}

bool Console::ExecuteConsoleCommand(std::string Command)
{
	std::vector<std::string> CommandVec = SeparateToStringArray(Command);
	if (!CommandVec.size())
	{
		ConsoleSystem->Print("Expected a Command.", ErrorLevel::Error);
		return false;
	}

	if (CommandVec.size() == 1 && ConsoleSystem->ConVars.contains(CommandVec[0]))
	{
		std::string ValueString;
		Variable FoundVar = ConsoleSystem->ConVars[CommandVec[0]];
		ConsoleSystem->Print(FoundVar.Name + " = " + GetVariableValueString(FoundVar));
		return true;
	}

	// Check if we are assigning a ConVar by checking if [0] is a ConVar and [1] is '='
	// shadow_resolution = 2048
	//       ^           ^   ^
	//      [0]         [1] [2]
	if (CommandVec.size() >= 2 && ConsoleSystem->ConVars.contains(CommandVec[0]) && CommandVec[1] == "=")
	{
		if (CommandVec.size() == 2)
		{
			ConsoleSystem->Print("Expected assignment.", ErrorLevel::Error);
			return false;
		}

		if (CommandVec.size() > 3)
		{
			ConsoleSystem->Print("Too many arguments. Expected one for assignment.", ErrorLevel::Error);
			return false;
		}

		auto& FoundVar = ConsoleSystem->ConVars[CommandVec[0]];
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
			ConsoleSystem->Print("Invalid type. Expected " + NativeType::TypeStrings[FoundVar.NativeType] + " (" + std::string(e.what()) + ")", ErrorLevel::Error);
			return false;
		}
		ConsoleSystem->Print("Assigned " + FoundVar.Name + " = " + CommandVec[2]);
		if (FoundVar.OnSet)
		{
			FoundVar.OnSet();
		}
		return true;
	}


	// Check if the first part of the command is a registered command
	if (ConsoleSystem->Commands.contains(CommandVec[0]))
	{
		// If any part of the command arguments is a ConVar, we replace it with it's value
		for (auto& i : CommandVec)
		{
			if (!ConsoleSystem->ConVars.contains(i))
			{
				continue;
			}
			const Variable& ConVar = ConsoleSystem->ConVars[i];
			i = GetVariableValueString(ConVar);
		}

		auto& FoundCommand = ConsoleSystem->Commands[CommandVec[0]];
		ConsoleSystem->ConsoleInput = std::vector<std::string>(CommandVec.begin() + 1, CommandVec.end());

		if (CommandVec.size() - 1 > FoundCommand.Arguments.size())
		{
			ConsoleSystem->Print(Command + ": Too many arguments.", ErrorLevel::Error);
			ConsoleSystem->PrintArguments(FoundCommand.Arguments, ErrorLevel::Error);
			return false;
		}

		for (size_t i = 0; i < FoundCommand.Arguments.size(); i++)
		{
			if (!FoundCommand.Arguments[i].Optional && (int)CommandVec.size() < i + 2)
			{
				ConsoleSystem->Print(Command + ": Not enough arguments.", ErrorLevel::Error);
				ConsoleSystem->PrintArguments(FoundCommand.Arguments, ErrorLevel::Error);
				return false;
			}
			if (FoundCommand.Arguments[i].Optional && CommandVec.size() < i + 2)
			{
				break;
			}

			if (IsTypeNumber(FoundCommand.Arguments[i].NativeType) && !IsTypeNumber(GetDataTypeFromString(CommandVec[i + 1])))
			{
				ConsoleSystem->Print("Expected "
					+ NativeType::TypeStrings[FoundCommand.Arguments[i].NativeType]
					+ " for '"
					+ FoundCommand.Arguments[i].Name
					+ "', found string.", ErrorLevel::Error);
				ConsoleSystem->PrintArguments(FoundCommand.Arguments, ErrorLevel::Error);
				return false;
			}
		}

		FoundCommand.Function();
		return true;
	}

	if (CommandVec.size())
	{
		ConsoleSystem->Print("Unknown ConVar or Command: " + CommandVec[0], ErrorLevel::Error);
	}
	return false;
}

Console::Command::Command(std::string Name, std::function<void()> Function, std::vector<Argument> Arguments)
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
			ConsoleSystem->Print("Error while registering new console command: " + NewCommand.Name + ". All optional arguments should be last.", ErrorLevel::Error);
			ConsoleSystem->Print("However, " + i.Name + " is not optional but the previous argument was.", ErrorLevel::Error);
			return;
		}
	}

	Commands.insert(std::pair(NewCommand.Name, NewCommand));
}

std::vector<std::string> Console::CommandArgs()
{
	return ConsoleInput;
}

void Console::PrintArguments(std::vector<Command::Argument> args, ErrorLevel Severity)
{
	if (!args.size())
	{
		ConsoleSystem->Print("Arguments are: none.", Severity);
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
	ConsoleSystem->Print(PrintArgs, Severity);
}

void Console::Update()
{
	// On linux, reading console input using std::cin on a different thread causes the entire terminal to freeze sometimes (what???)
	// But on windows this works fine.
#if _WIN32
	std::deque<std::string> ConsoleCommands;
	{
		std::unique_lock Lock{ ConsoleReadMutex };
		if (ConsoleConditionVariable.wait_for(Lock, std::chrono::seconds(0), [] { return !ConsoleLines.empty(); }))
		{
			std::swap(ConsoleLines, ConsoleCommands);
		}
	}


	for (auto& i : ConsoleCommands)
	{
		ExecuteConsoleCommand(i);
	}
#elif __linux
	// Most sane linux name (this is what the poll() argument is called)
	pollfd fds;
	fds.fd = fileno(stdin);
	fds.events = POLLIN;
	fds.revents = 0;

	int poll_result = poll(&fds, 1, 0);

	if (poll_result)
	{
		char Line[4000];
		std::cin.getline(Line, sizeof(Line));
		if (strlen(Line) > 0)
		{
			ExecuteConsoleCommand(Line);
		}
	}
#endif
}

Console::Command::Argument::Argument(std::string Name, NativeType::NativeType NativeType, bool Optional)
{
	this->Name = Name;
	this->NativeType = NativeType;
	this->Optional = Optional;
}

Console::Variable::Variable(std::string Name, NativeType::NativeType NativeType, void* Var, std::function<void()> OnSet)
{
	this->Name = Name;
	this->NativeType = NativeType;
	this->Var = Var;
	this->OnSet = OnSet;
}

#if _WIN32
void Console::ReadConsole()
{
	while (true)
	{
		std::string ReadString;
		std::getline(std::cin, ReadString);
		std::lock_guard lock{ ConsoleReadMutex };
		ConsoleLines.push_back(std::move(ReadString));
		ConsoleConditionVariable.notify_one();
	}
}
#endif

NativeType::NativeType Console::GetDataTypeFromString(const std::string& str)
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
