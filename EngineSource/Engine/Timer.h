#pragma once
#include <vector>

namespace Timer
{
	void StartTimer(void (*Function)(), float Time);

	namespace Internal
	{
		struct Timer
		{
			void (*Function)();
			float RemainingTime;
			Timer(void (*FuncIn)(), float Time);
		};
		extern std::vector<Timer> Timers;
	}
}