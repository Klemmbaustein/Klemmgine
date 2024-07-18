#include "BackgroundTask.h"
#include <cassert>
#include <Engine/Log.h>
#include <iostream>

thread_local BackgroundTask* BackgroundTask::ThisThreadPtr = nullptr;
std::string BackgroundTask::CurrentTaskStatus;
float BackgroundTask::CurrentTaskProgress;
bool BackgroundTask::IsRunningTask;
std::vector<BackgroundTask*> BackgroundTask::AllTasks;

BackgroundTask::BackgroundTask(std::function<void()> Function, std::function<void()> CallbackFunction)
{
	// Using a pointer as an ID. Why not?
	AllTasks.push_back(this);
	this->Callback = CallbackFunction;

	Thread = new std::thread(TaskRun, Function, this);
}

BackgroundTask::~BackgroundTask()
{
	Thread->join();
	delete Thread;
}

void BackgroundTask::SetProgress(float Progress)
{
	assert(ThisThreadPtr);
	ThisThreadPtr->Progress = Progress;
}

void BackgroundTask::SetStatus(std::string NewStatus)
{
	assert(ThisThreadPtr);
	ThisThreadPtr->Status = NewStatus;
}

void BackgroundTask::TaskRun(std::function<void()> Function, BackgroundTask* ThisTask)
{
	ThisThreadPtr = ThisTask;
	Function();
	ThisTask->Progress = 1;
}

bool BackgroundTask::IsBackgroundTask()
{
	return ThisThreadPtr;
}

BackgroundTaskSubsystem::BackgroundTaskSubsystem()
{
	Name = "BgTask";
}

void BackgroundTaskSubsystem::Update()
{
	BackgroundTask::IsRunningTask = false;
	for (size_t i = 0; i < BackgroundTask::AllTasks.size(); i++)
	{
		if (BackgroundTask::AllTasks[i]->Progress >= 1)
		{
			auto Callback = BackgroundTask::AllTasks[i]->Callback;
			delete BackgroundTask::AllTasks[i];
			BackgroundTask::AllTasks.erase(BackgroundTask::AllTasks.begin() + i);
			if (Callback)
			{
				Callback();
			}
			break;
		}
		else if (!BackgroundTask::IsRunningTask)
		{
			BackgroundTask::CurrentTaskProgress = BackgroundTask::AllTasks[i]->Progress;
			BackgroundTask::CurrentTaskStatus = BackgroundTask::AllTasks[i]->Status;
			BackgroundTask::IsRunningTask = true;
		}
	}
}
