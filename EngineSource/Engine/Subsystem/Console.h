#pragma once
#include <string>
#include <Engine/TypeEnun.h>
#include <vector>
#include <set>
#include <map>
#include <thread>
#include <condition_variable>

#include "Subsystem.h"
#include <mutex>
#include <deque>

/**
* @file
*/

/**
* @brief
*/
class Console : public Subsystem
{
public:

	/// Initializes default ConVars and Commands
	Console();
	~Console() override;

	/**
	* @brief
	* Executes the given command string.
	* 
	* @return
	* True if the command string was evaluated successfully.
	*/
	static bool ExecuteConsoleCommand(std::string Command);

	static Console* ConsoleSystem;

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

	/// A function to print out a vector of command arguments. Useful for displaying info about a command
	/// or why a certain command has invalid arguments passed to it.
	void PrintArguments(std::vector<Command::Argument> args, ErrorLevel Severity = ErrorLevel::Info);

	void Update() override;

private:
#if _WIN32
	static std::condition_variable ConsoleConditionVariable;
	static std::mutex ConsoleReadMutex;
	static std::deque<std::string> ConsoleLines;

	std::thread ReadConsoleThread;
	static void ReadConsole();
#endif

	std::map<std::string, Variable> ConVars;
	std::map<std::string, Command> Commands;
	std::vector<std::string> ConsoleInput;

	static NativeType::NativeType GetDataTypeFromString(const std::string& str);
	static bool IsTypeNumber(NativeType::NativeType GivenType)
	{
		return GivenType == NativeType::Int || GivenType == NativeType::Float;
	}
	static std::vector<std::string> SeperateToStringArray(const std::string& InString);

	template<typename T>
	static void SafePtrAssign(void* Ptr, T NewValue)
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
};