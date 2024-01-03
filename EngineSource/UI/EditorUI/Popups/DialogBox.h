#if EDITOR
#pragma once
#include "EditorPopup.h"

class DialogBox : public EditorPopup
{
public:
	DialogBox(std::string Title, Vector2 Position, std::string Message, std::vector<PopupOption> Answers);
	~DialogBox();
	void OnButtonClicked(int Index) override;
	void Tick() override;
};
#endif