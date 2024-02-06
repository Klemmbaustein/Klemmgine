#pragma once
#include <string>
#include <Engine/Utility/FileUtility.h>

#define FILENAME FileUtil::GetFileNameFromPath(__FILE__)

#define FILEPOS (FILENAME + ", " + std::string(__FUNCTION__) + ", Line " + std::to_string(__LINE__))

/**
* @brief
* 
*/
namespace Error
{
	void Init();
	
	void AssertFailure(std::string Name, std::string Location);
	
	inline void Assert(bool Value, std::string Name, std::string Location)
	{
		if (!Value)
		{
			AssertFailure(Name, Location);
		}
	}

	void PrintStackTrace();
}

/**
* @file
* Contains functions related to error handling.
*/

/**
* @def ENGINE_ASSERT(Condition, Description)
* @brief
* Evaluates @a Condition. if it evaluates to false, it crashes the engine and displays a stack trace if possible.
* 
* @param Condition
* The condition to check.
* 
* @param Description
* A description of the condition/crash that will be displayed if the condition fails.
*/
#define ENGINE_ASSERT(Condition, Description) Error::Assert(Condition, Description + std::string("\nCondition: ") + #Condition + std::string(""), FILEPOS)

/**
* @def ENGINE_UNREACHABLE()
* @brief
* Marks this code as unreachable. The engine will crash if the code is reached.
*/
#define ENGINE_UNREACHABLE() Error::AssertFailure("Unreachable code reached. This should never happen.", FILEPOS)