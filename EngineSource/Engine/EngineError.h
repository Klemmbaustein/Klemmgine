#pragma once
#include <string>

#if _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

namespace Engine
{
	void AssertFailure(std::string Name, std::string Location);
	
	inline void Assert(bool Value, std::string Name, std::string Location)
	{
		if (!Value)
		{
			AssertFailure(Name, Location);
		}
	}
}

// Crashes everything if the condition evaluates to false.
#define ENGINE_ASSERT(Cond, Description) Engine::Assert(Cond, Description + std::string("\nCondition: ") + #Cond + std::string(""), std::string("File: ") + __FILENAME__ + ", Line " + std::to_string(__LINE__))
