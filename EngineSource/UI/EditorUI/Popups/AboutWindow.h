#if EDITOR
#pragma once
#include <UI/EditorUI/Popups/EditorPopup.h>

class AboutWindow : public EditorPopup
{
public:
	UIBox* ContentBox = nullptr;
	AboutWindow();
	~AboutWindow();
	void OnButtonClicked(int Index) override;
	void Tick() override;
protected:
};

#endif