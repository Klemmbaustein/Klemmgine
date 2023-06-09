#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>

class StatusBar : public EditorPanel
{
	UIText* Texts[3];
	float FPSUpdateTimer = 1;
	unsigned int DisplayedFPS = 60;
public:
	StatusBar(Vector3* Colors);

	void UpdateLayout() override;
	void Tick() override;
	void OnButtonClicked(int Index) override;
};

#endif