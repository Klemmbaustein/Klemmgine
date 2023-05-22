#if !RELEASE && !EDITOR
#pragma once
#include <UI/Default/UICanvas.h>
#include <UI/UIfwd.h>

class DebugUI : public UICanvas
{
	int IsEditingText = 5;
	size_t LogLength = 0;
	TextRenderer* Text = nullptr;
	UITextField* LogPrompt = nullptr;
	UIText* DebugTexts[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };

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