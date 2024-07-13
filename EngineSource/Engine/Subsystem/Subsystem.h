#pragma once
#include <string>
#include <vector>

/**
* @defgroup Internal Internal
*
* Internal engine systems that a normal project won't use.
*/

/**
* @defgroup Subsystem Subsystem
* 
* Subsystems controlling one feature of the engine.
* 
* @ingroup Internal
*/


/**
 * @brief
 * A Subsystem controlling one feature of the engine.
 * 
 * @ingroup Subsystem
 */
class Subsystem
{
protected:
	static std::vector<Subsystem*> LoadedSystems;
	std::string Prefix;
	static bool IsVerbose;
public:

	static void SetSystemLogVerbose(bool NewIsVerbose);

	/// The name of the subsystem.
	const char* Name = "sys";
	/// The type of the subsystem. If this is empty, there is no type.
	const char* SystemType = "";
	
	Subsystem();
	Subsystem(const Subsystem&) = delete;
	virtual ~Subsystem();

	virtual void Update();

	/// Error level used by Subsystem::Print()
	enum class ErrorLevel
	{
		/// Info level. White color.
		Info,
		/// Warn level. Yellow color.
		Warn,
		/// Error level. Red color.
		Error,
		/// Note level. Gray color, only printed if the command arguments contain -verbose.
		Note
	};

	/**
	 * @brief
	 * Prints a message as the subsystem.
	 * 
	 * The message is formatted as:
	 * 
	 * `[<SystemType>]: [<Name>]: <Msg>`
	 * or if SystemType is empty:
	 * `[<Name>]: <Msg>`
	 */
	void Print(std::string Msg, ErrorLevel MsgErrLvl = ErrorLevel::Info);

	/// Registers and loads a new Subsystem.
	static void Load(Subsystem* System);
	/// Unloads an already loaded Subsystem.
	static void Unload(Subsystem* System);

	/**
	 * @brief
	 * Returns the subsystem with the given name. If no such Subsystem exists, this function returns nullptr.
	 */
	static Subsystem* GetSubsystemByName(std::string Name);

	static void UpdateSubsystems();

	static void DestroyAll();
};