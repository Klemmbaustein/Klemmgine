#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>

class StatusBar : public EditorPanel
{
	UIText* StatusText;
	UIBox* BarBoxes[2];
	float FPSUpdateTimer = 1;
	unsigned int DisplayedFPS = 60;
	int Selected = 0;
public:
	StatusBar(Vector3* Colors);

	void GenerateWindowButtons(std::vector<int> ButtonIndices);

	UIBox* WindowButtonBox = nullptr;

	UIBox* MenuBarDropdown = nullptr;

	static bool IsHovered();

	void GenerateMenuBarDropdown(int ButtonIndex);

	void UpdateLayout() override;
	void Tick() override;
	void OnButtonClicked(int Index) override;
};

#endif