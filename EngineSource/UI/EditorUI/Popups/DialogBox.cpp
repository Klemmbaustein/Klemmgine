#if EDITOR
#include "DialogBox.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>

DialogBox::DialogBox(std::string Title, Vector2 Position, std::string Message, std::vector<PopupOption> Answers)
	: EditorPopup(Position, 0.3f, Title)
{
	PopupBackground->AddChild((new UIText(0.45f, EditorUI::UIColors[2], Message, EditorUI::Text))
		->SetWrapEnabled(true, (PopupBackground->GetMinSize().X - 0.1f), UIBox::SizeMode::ScreenRelative)
		->SetPadding(0.01f));
	SetOptions(Answers);
}

DialogBox::~DialogBox()
{
}

void DialogBox::OnButtonClicked(int Index)
{
	HandlePopupButtons(Index);
}

void DialogBox::Tick()
{
	TickPopup();
}
#endif