#pragma once
#include <string>
#include <Engine/TypeEnun.h>
#include <vector>

namespace Console
{
	// Initialized default ConVars and Commands
	void InitializeConsole();

	// Executes the given command string. If verbose, additional info will be displayed.
	// This function will return true if the command string was evaluated successfully. Otherwise it will return false.
	bool ExecuteConsoleCommand(std::string Command, bool Verbose = false);

	// ConVar struct.
	struct Variable
	{
		std::string Name;
		// If OnSet == nullptr/NULL, OnSet will be ignored.
		void(*OnSet)() = nullptr;
		void* Var = nullptr;
		Type::TypeEnum Type = Type::E_INT;
		Variable(std::string Name, Type::TypeEnum Type, void* Var, void (*OnSet)());
		Variable() {}
	};

	// Command struct.
	struct Command
	{
		std::string Name;
		void(*Function)() = nullptr;
		struct Argument
		{
			Argument(std::string Name, Type::TypeEnum Type, bool Optional = false);
			std::string Name;
			Type::TypeEnum Type;
			bool Optional;
		};
		std::vector<Argument> Arguments;
		
		Command(std::string Name, void(*Function)(), std::vector<Argument> Arguments);
		Command() {}
	};

	// Registers a ConVar that can be modified through the console.
	void RegisterConVar(Variable Var);
	// Registers a Command that can be called through the console.
	void RegisterCommand(Command NewCommand);

	// Gets the arguments passed to the most recent command. This is totally necessary, trust me.
	std::vector<std::string> CommandArgs();

	enum ConsoleLogType
	{
		E_INFO = 0,
		E_WARNING = 1,
		E_ERROR = 2
	};

	// Logging function for the console. It is basically just Log::Print() but with preset colors.
	void ConsoleLog(std::string Message, ConsoleLogType Severity = E_INFO);

	// A function to print out a vector of command arguments. Useful for displaying info about a command
	// or why a certain command has invalid arguments passed to it.
	void PrintArguments(std::vector<Command::Argument> args, ConsoleLogType Severity = E_INFO);
}