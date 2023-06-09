#include "EngineError.h"
#include <Engine/Log.h>

void Engine::AssertFailure(std::string Name, std::string Location)
{
	Log::Print("[Error]: --------------------------------[Assert failed]--------------------------------", Log::LogColor::Red);
	Log::PrintMultiLine(Name, Log::LogColor::Red, "[Error]: ");
	Log::Print("[Error]: " + Location, Log::LogColor::Red);

	throw "Assert failed.";
}
