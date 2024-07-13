#pragma once
#include "Subsystem.h"
#include <fstream>
#include <thread>
#include <atomic>
#include <Engine/Log.h>

/**
* @brief
* Subsystem responsible for writing log files.
* 
* @ingroup Internal
*/
class LogSubsystem : public Subsystem
{
public:
	LogSubsystem();
	~LogSubsystem();

	void Update();

	/**
	* @brief
	* Writes all messages to the log file instantly.
	*/
	static void Flush();

private:

	static void WriteLogFile(std::ofstream* File);
	static void WriteSection(const std::vector<Log::Message>& Messages, std::ofstream* File);
	static std::thread LogWriteThread;
	std::ofstream LogFile;
	static size_t LastWritten;
	static std::atomic<bool> WriteLog;
};