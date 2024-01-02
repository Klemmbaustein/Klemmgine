#if EDITOR && 0
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class AboutWindow : public EditorPanel
{
public:
	UIBox* ContentBox = nullptr;
	AboutWindow();
	void UpdateLayout() override;
	~AboutWindow();
	void OnButtonClicked(int Index) override;
	void Tick() override;
protected:
};

#endif