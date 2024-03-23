#pragma once
#include "Subsystem.h"
#include <fstream>
#include <thread>
#include <atomic>
#include <Engine/Log.h>

class LogSubsystem : public Subsystem
{
public:
	LogSubsystem();
	~LogSubsystem();

	void Update();

	static void Flush();

	static void WriteLogFile(std::ofstream* File);
	static void WriteSection(const std::vector<Log::Message>& Messages, std::ofstream* File);

private:
	static std::thread LogWriteThread;
	std::ofstream LogFile;
	static size_t LastWritten;
	static std::atomic<bool> WriteLog;

};