#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class UITextField;
class UIText;

class LogUI : public EditorPanel
{
	std::vector<UIText*> LogTexts;
public:
	LogUI(Vector3* UIColors, Vector2 Position, Vector2 Scale);
	void Save() override;
	void Load(std::string File) override;
	void UpdateLayout() override;

	UITextField* LogPromt = nullptr;
	UIScrollBox* LogScrollBox;
	size_t PrevLogLength = 0;
	size_t PrevAmount = 0;
	void OnButtonClicked(int Index) override;

	void Tick() override;
};

#endif