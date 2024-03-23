#define __STDC_WANT_LIB_EXT1__ 1
#include "LogSubsystem.h"
#include <ctime>
#include <Engine/EngineProperties.h>
#include <filesystem>

size_t LogSubsystem::LastWritten = 0;
std::atomic<bool> LogSubsystem::WriteLog = true;
std::thread LogSubsystem::LogWriteThread;

LogSubsystem::LogSubsystem()
{
	Name = "LogIOSys";
	std::string Date;

	WriteLog = true;

	time_t CurrentTime = std::time(nullptr);

	char TimeStringBuffer[128];

	tm CurrentTimeTm;

	localtime_s(&CurrentTimeTm, &CurrentTime);

	std::strftime(TimeStringBuffer, sizeof(TimeStringBuffer), "%Y-%m-%d %H-%M-%S", &CurrentTimeTm);

	std::filesystem::create_directory("Logs/");


#if EDITOR
	std::string Config = "Editor";
#elif RELEASE
	std::string Config = "Release";
#elif SERVER
	std::string Config = "Server";
#else
	std::string Config = "Debug";
#endif

	LogFile = std::ofstream("Logs/" + std::string(Project::ProjectName) + "-" + Config + "-" + std::string(TimeStringBuffer) + ".txt");

	LogFile << "-- Klemmgine " << VERSION_STRING << "-" << Config << " log file. Written: " << TimeStringBuffer << " --" << std::endl;

	LogWriteThread = std::thread(WriteLogFile, &LogFile);
}

LogSubsystem::~LogSubsystem()
{
	Flush();
}

void LogSubsystem::Update()
{
}

void LogSubsystem::Flush()
{
	WriteLog = false;
	LogWriteThread.join();
}

void LogSubsystem::WriteLogFile(std::ofstream* File)
{
	uint8_t CheckTimer = 0;
	while (WriteLog)
	{
		if (CheckTimer == 0)
		{
			auto LogMessages = Log::GetMessages();

			if (LogMessages.size() != LastWritten)
			{
				WriteSection(LogMessages, File);
			}
		}
		CheckTimer++;
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	{
		auto LogMessages = Log::GetMessages();
		WriteSection(LogMessages, File);
		*File << "-- LogSubsystem destroyed --";
		File->close();
	}
}

void LogSubsystem::WriteSection(const std::vector<Log::Message>& Messages, std::ofstream* File)
{
	for (; LastWritten < Messages.size(); LastWritten++)
	{
		*File << Messages[LastWritten].Text << std::endl;
	}
}
