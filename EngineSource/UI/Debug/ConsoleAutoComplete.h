#if !SERVER
#pragma once
#include <UI/UICanvas.h>
#include <UI/UIText.h>
#include <Engine/Subsystem/Console.h>

namespace Debug
{
	class ConsoleAutoComplete
	{
		TextRenderer* Font = nullptr;
		float TextSize = 0;
	public:
		size_t SelectionIndex = 0;

		ConsoleAutoComplete(TextRenderer* Font, float TextSize);

		std::string CompleteSelection(std::string Command);

		struct Recommendation
		{
			std::string Type;
			std::string Name;
			bool IsComplete = false;
			size_t NameHighlight = 0;
			std::vector<std::string> Attributes;
		};

		std::vector<Recommendation> GetRecommendations(std::string Command);
		std::vector<std::string> GetCommandArguments(const Console::Command& Command);

		void RenderToBox(UIBox* Target, const std::vector<Recommendation>& Found);
	};
}
#endif