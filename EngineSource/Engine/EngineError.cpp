#include "EngineError.h"
#include <Engine/Log.h>
#include <stacktrace>
#include <iostream>
#include <csignal>
#include <Engine/OS.h>
#include <Engine/Stats.h>

static void HandleSigSegV(int SignalID)
{
	Error::AssertFailure("Access violation", "Status: " + Debugging::EngineStatus);
}

void Error::Init()
{
	signal(SIGSEGV, HandleSigSegV);
}

void Error::AssertFailure(std::string Name, std::string Location)
{
	OS::SetConsoleWindowVisible(true);
	Log::Print("[Error]: ------------------------------------[Error]------------------------------------", Log::LogColor::Red);
	Log::PrintMultiLine(Name, Log::LogColor::Red, "[Error]: ");
	Log::Print("[Error]: " + Location, Log::LogColor::Red);

	PrintStackTrace();

#if _WIN32
	__debugbreak();
#else
	throw 0;
#endif
	exit(1);
}

void Error::PrintStackTrace()
{
#if __cpp_lib_stacktrace >= 202011L
	Log::Print("[Error]: ---------------------------------[Stack trace]---------------------------------", Log::LogColor::Red);
	auto trace = std::stacktrace::current();
	for (const auto& i : trace)
	{
#ifdef _MSC_VER
		std::string descr = i.description();
		descr = descr.substr(descr.find_first_of("!") + 1);
		descr = descr.substr(0, descr.find_first_of("+"));
		descr.append("(), Line " + std::to_string(i.source_line()));
		if (i.source_file() == __FILE__)
		{
			continue;
		}
#else
		std::string descr = i.description();
#endif
		if (FileUtil::GetExtension(i.source_file()) == "inl")
		{
			continue;
		}

		if (i.source_file().empty())
		{
			continue;
		}

		Log::Print("[Error]:\tat: " + FileUtil::GetFileNameFromPath(i.source_file()) + ": " + descr, Log::LogColor::Red);
	}
#endif

}
