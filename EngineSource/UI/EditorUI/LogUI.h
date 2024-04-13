#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class UITextField;
class UIText;

/**
* @brief
* Console panel in the editor.
* 
* @ingroup Editor
*/
class LogUI : public EditorPanel
{
	std::vector<UIText*> LogTexts;

	void UpdateLogBoxSize();
public:
	LogUI(EditorPanel* Parent);
	void OnResized() override;

	static void PrintUIElement(UIBox* Element);
	void ResetScroll();

	UITextField* LogPrompt = nullptr;
	UIScrollBox* LogScrollBox;
	size_t PrevLogLength = 0;
	int PrevAmount = 0;
	void OnButtonClicked(int Index) override;

	void Tick() override;
};

#endif