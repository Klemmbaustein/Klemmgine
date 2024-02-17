#include "BackgroundTask.h"
#include <cassert>
#include "Log.h"
#include <iostream>

thread_local BackgroundTask* BackgroundTask::ThisThreadPtr = nullptr;
std::string BackgroundTask::CurrentTaskStatus;
float BackgroundTask::CurrentTaskProgress;
bool BackgroundTask::IsRunningTask;
std::vector<BackgroundTask*> BackgroundTask::AllTasks;

BackgroundTask::BackgroundTask(void(*Function)(), void(*Callback)())
{
	// Using a pointer as an ID. Why not?
	NativeType = (size_t)Function;
	AllTasks.push_back(this);
	this->Callback = Callback;

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

void BackgroundTask::TaskRun(void (*Function)(), BackgroundTask* ThisTask)
{
	ThisThreadPtr = ThisTask;
	Function();
	ThisTask->Progress = 1;
}

bool BackgroundTask::IsFunctionRunningAsTask(void(*Function)())
{
	for (auto& i : AllTasks)
	{
		if (i->NativeType == (size_t)Function)
		{
			return true;
		}
	}
	return false;
}

bool BackgroundTask::IsBackgroundTask()
{
	return ThisThreadPtr;
}

void BackgroundTask::UpdateTaskStatus()
{
	IsRunningTask = false;
	for (size_t i = 0; i < AllTasks.size(); i++)
	{
		if (AllTasks[i]->Progress >= 1)
		{
			auto Callback = AllTasks[i]->Callback;
			delete AllTasks[i];
			AllTasks.erase(AllTasks.begin() + i);
			if (Callback)
			{
				Callback();
			}
			break;
		}
		else if (!IsRunningTask)
		{
			CurrentTaskProgress = AllTasks[i]->Progress;
			CurrentTaskStatus = AllTasks[i]->Status;
			IsRunningTask = true;
		}
	}
}