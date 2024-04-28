#include "EngineError.h"
#include <Engine/Log.h>
#include <iostream>
#include <Engine/Application.h>
#include <csignal>
#include <Engine/OS.h>
#include <SDL.h>
#include <Engine/Stats.h>
#include <Engine/Subsystem/LogSubsystem.h>
#include <array>
#include <map>

#if __cpp_lib_stacktrace >= 202011L
#include <stacktrace>
#endif

std::map<int, const char*> SignalTypes =
{
	{SIGABRT, "SIGABRT"},
	{SIGFPE, "Math error"},
	{SIGILL, "Illegal instruction"},
	{SIGINT, "SIGINT"},
	{SIGSEGV, "Access violation"},
	{SIGTERM, "SIGTERM"},
};

static bool Failed = false;

static void HandleSignal(int SignalID)
{
	if (Failed)
	{
		return;
	}
	Failed = true;

	Error::AssertFailure(SignalTypes[SignalID], Stats::EngineStatus);
}

void Error::Init()
{
	signal(SIGABRT, HandleSignal);
	signal(SIGFPE, HandleSignal);
	signal(SIGILL, HandleSignal);
	signal(SIGSEGV, HandleSignal);
	signal(SIGTERM, HandleSignal);
}

void Error::AssertFailure(std::string Name, std::string Position)
{
	OS::SetConsoleWindowVisible(true);
	SDL_DestroyWindow(Application::Window);
	Log::Print("[Error]: ------------------------------------[Error]------------------------------------", Log::LogColor::Red);
	Log::PrintMultiLine(Name, Log::LogColor::Red, "[Error]: ");
	Log::Print("[Error]: " + Position, Log::LogColor::Red);

	PrintStackTrace();

	LogSubsystem::Flush();
	abort();
}

void Error::PrintStackTrace()
{
#if __cpp_lib_stacktrace >= 202011L
	auto trace = std::stacktrace::current();
	if (trace.empty())
	{
		return;
	}
	Log::Print("[Error]: ---------------------------------[Stack trace]---------------------------------", Log::LogColor::Red);
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
