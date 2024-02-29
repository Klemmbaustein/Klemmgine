#pragma once
#include <string>
#include <Engine/TypeEnun.h>
#include <vector>

/**
* @file
*/

/**
* @brief
*/
namespace Console
{
	/// Initialized default ConVars and Commands
	void InitializeConsole();

	/**
	* @brief
	* Executes the given command string.
	* 
	* @return
	* True if the command string was evaluated successfully.
	*/
	bool ExecuteConsoleCommand(std::string Command);

	/// Contains information about a console variable.
	struct Variable
	{
		std::string Name;
		// If OnSet == nullptr/NULL, OnSet will be ignored.
		void(*OnSet)() = nullptr;
		void* Var = nullptr;
		NativeType::NativeType NativeType = NativeType::Int;
		Variable(std::string Name, NativeType::NativeType NativeType, void* Var, void (*OnSet)());
		Variable() {}
	};

	/// Contains information about a console command.
	struct Command
	{
		std::string Name;
		void(*Function)() = nullptr;
		struct Argument
		{
			Argument(std::string Name, NativeType::NativeType NativeType, bool Optional = false);
			std::string Name;
			NativeType::NativeType NativeType;
			bool Optional;
		};
		std::vector<Argument> Arguments;
		
		Command(std::string Name, void(*Function)(), std::vector<Argument> Arguments);
		Command() {}
	};

	/// Registers a ConVar that can be modified through the console.
	void RegisterConVar(Variable Var);
	/// Registers a Command that can be called through the console.
	void RegisterCommand(Command NewCommand);

	/// Gets the arguments passed to the most recent command.
	std::vector<std::string> CommandArgs();

	enum ConsoleLogType
	{
		Info = 0,
		Warn = 1,
		Err = 2
	};

	/// Logging function for the console. It is like Log::Print() but with preset colors.
	void ConsoleLog(std::string Message, ConsoleLogType Severity = Info);

	/// A function to print out a vector of command arguments. Useful for displaying info about a command
	/// or why a certain command has invalid arguments passed to it.
	void PrintArguments(std::vector<Command::Argument> args, ConsoleLogType Severity = Info);
}