#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class UITextField;
class UIText;

class LogUI : public EditorPanel
{
	std::vector<UIText*> LogTexts;

	void UpdateLogBoxSize();
public:
	LogUI(Vector3* UIColors, Vector2 Position, Vector2 Scale);
	void UpdateLayout() override;

	UITextField* LogPrompt = nullptr;
	UIScrollBox* LogScrollBox;
	size_t PrevLogLength = 0;
	int PrevAmount = 0;
	void OnButtonClicked(int Index) override;

	void Tick() override;
};

#endif