#include "Timer.h"

void Timer::StartTimer(void(*Function)(), float Time)
{
	Timer::Internal::Timers.push_back(Timer::Internal::Timer(Function, Time));
}

Timer::Internal::Timer::Timer(void(*FuncIn)(), float Time)
{
	Function = FuncIn;
	RemainingTime = Time;
}

std::vector<Timer::Internal::Timer> Timer::Internal::Timers;
