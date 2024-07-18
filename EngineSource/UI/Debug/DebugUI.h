#if !RELEASE && !EDITOR && !SERVER
#pragma once
#include <UI/UICanvas.h>
#include <UI/UIfwd.h>
#include <Engine/Input.h>
#include "ConsoleAutoComplete.h"

namespace Debug
{
	class DebugUI : public UICanvas
	{
		ConsoleAutoComplete* AutoComplete = nullptr;
		std::string LastCommand;
		float StatsRedrawTimer = 0;
		unsigned int FPS = 0;
		int IsEditingText = 5;
		size_t LogLength = 0;
		TextRenderer* Text = nullptr;
		UIBackground* CompleteBackground = nullptr;
		UITextField* LogPrompt = nullptr;
		UIText* DebugTexts[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
		UIBox* PerformanceGraphBox = nullptr;

		bool ConsoleReadInput(Input::Key Key);
		void UpdateAutoComplete();

		bool RShiftDown = false;
		int LastLogMessageAmount = 0;
		bool UpDownPressed = false;
		void UpdatePerfGraph();
		UIBackground* LogBackground = nullptr;
	public:
		static DebugUI* CurrentDebugUI;
		DebugUI();
		void Tick() override;
		virtual ~DebugUI();
		void GenerateLog();

		void OnButtonClicked(int Index) override;
	};
}

#endif