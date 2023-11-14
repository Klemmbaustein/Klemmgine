#if EDITOR
#include "DialogBox.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>

DialogBox::DialogBox(std::string Title, Vector2 Position, std::string Message, std::vector<Answer> Answers)
	: EditorPanel(Editor::CurrentUI->UIColors, Position, Vector2(0.4f, 0.15f), Vector2(0.25f, 0.15f), 2, true, Title)
{
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetVerticalAlign(UIBox::Align::Centered);
	ButtonBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);
	TabBackground->SetVerticalAlign(UIBox::Align::Default);
	TabBackground->AddChild(ButtonBackground);
	this->Answers = Answers;
	for (size_t i = 0; i < Answers.size(); i++)
	{
		ButtonBackground->AddChild(
			(new UIButton(true, 0, UIColors[2], this, (int)i))
			->SetPadding(0.01f)
			->SetBorder(UIBox::BorderType::Rounded, 0.2f)
			->AddChild((new UIText(0.45f, 1 - UIColors[2], Answers[i].Name, Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f)));
	}
	TabBackground->AddChild(new UIText(0.5f, UIColors[2], Message, Editor::CurrentUI->EngineUIText));
	UpdateLayout();
}

void DialogBox::UpdateLayout()
{

	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075f));
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