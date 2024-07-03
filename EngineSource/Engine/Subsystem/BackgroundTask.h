#pragma once
#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include <functional>
#include "Subsystem.h"

struct BackgroundTask
{
	std::atomic<float> Progress = 0;
	std::string Status;
	std::thread* Thread = nullptr;
	std::function<void()> Callback = nullptr;

	// Will point to the task that is being executed right now.
	thread_local static BackgroundTask* ThisThreadPtr;

	BackgroundTask(std::function<void()> Function, std::function<void()> Callback = nullptr);
	~BackgroundTask();

	static void SetProgress(float Progress);
	static void SetStatus(std::string NewStatus);

	static std::string CurrentTaskStatus;
	static float CurrentTaskProgress;
	static bool IsRunningTask;

	static bool IsBackgroundTask();

	static std::vector<BackgroundTask*> AllTasks;

private:
	static void TaskRun(std::function<void()> Function, BackgroundTask* Task);
};

class BackgroundTaskSubsystem : public Subsystem
{
public:

	BackgroundTaskSubsystem();

	void Update() override;
};