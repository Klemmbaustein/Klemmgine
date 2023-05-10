#if EDITOR
#include "DialogBox.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>

DialogBox::DialogBox(std::string Title, Vector2 Position, std::string Message, std::vector<Answer> Answers)
	: EditorPanel(Editor::CurrentUI->UIColors, Position, Vector2(0.4, 0.15), Vector2(0.25, 0.15), 2, true, Title)
{
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetBorder(UIBox::E_DARKENED_EDGE, 0.2);
	TabBackground->Align = UIBox::E_DEFAULT;
	TabBackground->AddChild(ButtonBackground);
	this->Answers = Answers;
	for (size_t i = 0; i < Answers.size(); i++)
	{
		ButtonBackground->AddChild(
			(new UIButton(true, 0, UIColors[2], this, i))
			->SetPadding(0.01)
			->SetBorder(UIBox::E_ROUNDED, 0.2)
			->AddChild((new UIText(0.45, 1 - UIColors[2], Answers[i].Name, Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005)));
	}
	TabBackground->AddChild(new UIText(0.5, UIColors[2], Message, Editor::CurrentUI->EngineUIText));
	UpdateLayout();
}

void DialogBox::UpdateLayout()
{
	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075));
}

DialogBox::~DialogBox()
{
}

void DialogBox::OnButtonClicked(int Index)
{
	if (Answers.at(Index).OnPressed)
	{
		Answers.at(Index).OnPressed();
	}
	delete this;
	return;
}

void DialogBox::Tick()
{
	UpdatePanel();
}
#endif