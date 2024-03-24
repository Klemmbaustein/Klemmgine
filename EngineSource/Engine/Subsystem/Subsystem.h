#pragma once
#include <string>
#include <vector>

class Subsystem
{
protected:
	static std::vector<Subsystem*> LoadedSystems;
	std::string Prefix;
	static bool IsVerbose;
public:

	static void SetSysemLogVerbose(bool NewIsVerbose);

	const char* Name = "sys";
	const char* SystemType = "";
	
	Subsystem();
	virtual ~Subsystem();

	virtual void Update();

	enum class ErrorLevel
	{
		Info,
		Warn,
		Error,
		Note
	};

	void Print(std::string Msg, ErrorLevel MsgErrLvl = ErrorLevel::Info);

	static void Load(Subsystem* System);
	static void Unload(Subsystem* System);
	static Subsystem* GetSubsystemByName(std::string Name);

	static void UpdateSubsystems();

	static void DestroyAll();
};