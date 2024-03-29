#if !RELEASE && !EDITOR
#pragma once
#include <UI/Default/UICanvas.h>
#include <UI/UIfwd.h>
#include <Engine/Input.h>

class DebugUI : public UICanvas
{
	float StatsRedrawTimer = 0;
	unsigned int FPS = 0;
	int IsEditingText = 5;
	size_t LogLength = 0;
	TextRenderer* Text = nullptr;
	UITextField* LogPrompt = nullptr;
	UIText* DebugTexts[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	bool ConsoleReadInput(Input::Key Key);

	int LastLogMessageAmount = 0;

	UIBackground* LogBackground = nullptr;
public:
	static DebugUI* CurrentDebugUI;
	DebugUI();
	void Tick() override;
	virtual ~DebugUI();
	void GenerateLog();

	void OnButtonClicked(int Index);
};

#endif