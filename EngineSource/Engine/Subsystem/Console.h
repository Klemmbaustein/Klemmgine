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
#include <functional>
#include <deque>

/**
* @file
*/

/**
* @brief
* Console subsystem, responsible for handling the engine console.
* 
* @ingroup Subsystem
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

	/// The current active console subsystem.
	static Console* ConsoleSystem;

	/// Contains information about a console variable.
	struct Variable
	{
		/// Name of the console variable.
		std::string Name;
		/// Callback called when the value of the variable is changed.
		std::function<void()> OnSet = nullptr;
		/// A pointer to the variable in memory.
		void* Var = nullptr;
		/// The type of the variable.
		NativeType::NativeType NativeType = NativeType::Int;
		Variable(std::string Name, NativeType::NativeType NativeType, void* Var, std::function<void()> OnSet);
		Variable() {}
	};

	/// Contains information about a console command.
	struct Command
	{
		/// The name of the command.
		std::string Name;

		/// The function for the command. If the command is called, the function is called.
		std::function<void()> Function = nullptr;

		/// A console command argument.
		struct Argument
		{
			Argument(std::string Name, NativeType::NativeType NativeType, bool Optional = false);
			/// The name of the argument.
			std::string Name;
			/// The type of the argument value.
			NativeType::NativeType NativeType;
			/// True if the argument is optional. Optional arguments must always be the last.
			bool Optional;
		};

		/// The arguments for this command. If the arguments aren't satisfied, Function will not be called.
		std::vector<Argument> Arguments;
		
		Command(std::string Name, std::function<void()> Function, std::vector<Argument> Arguments);
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

	std::map<std::string, Variable> ConVars;
	std::map<std::string, Command> Commands;
	static std::vector<std::string> SeparateToStringArray(const std::string& InString);

	static std::string GetVariableValueString(Variable Var);

private:
#if _WIN32
	static std::condition_variable ConsoleConditionVariable;
	static std::mutex ConsoleReadMutex;
	static std::deque<std::string> ConsoleLines;

	std::thread ReadConsoleThread;
	static void ReadConsole();
#endif

	std::vector<std::string> ConsoleInput;

	static NativeType::NativeType GetDataTypeFromString(const std::string& str);
	static bool IsTypeNumber(NativeType::NativeType GivenType)
	{
		return GivenType == NativeType::Int || GivenType == NativeType::Float;
	}

	template<typename T>
	static void SafePtrAssign(void* Ptr, T NewValue)
	{
		T* ptr = (T*)Ptr;
		if (ptr)
		{
			*ptr = NewValue;
		}
	}
	const std::set<char> Separators
	{
		' ',
		'	',
	};

	const std::set<char> SpecialTokens
	{
		'='
	};
};