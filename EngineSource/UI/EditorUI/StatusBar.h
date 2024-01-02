#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>

/**
* @brief
* Status bar in the editor.
* 
* Displays 'File' 'Edit' 'View' 'Help', Memory usage and performance statistics.
* 
* @ingroup Editor
*/
class StatusBar : public UICanvas
{
	UIText* StatusText;
	UIBox* BarBoxes[2];
	float FPSUpdateTimer = 1;
	unsigned int DisplayedFPS = 60;
	int Selected = 0;
public:
	UIBackground* StatusBackground = nullptr;
	StatusBar();
	UIBox* WindowButtonBox = nullptr;

	UIBox* MenuBarDropdown = nullptr;

	void GenerateMenuBarDropdown(int ButtonIndex);

	void Tick() override;
	void OnButtonClicked(int Index) override;
};

#endif