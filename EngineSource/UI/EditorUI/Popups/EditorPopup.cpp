#if EDITOR
#include "EditorPopup.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Input.h>

static EditorPopup* DraggedPopup = nullptr;
static Vector2 DraggedOffset = 0;

void EditorPopup::SetOptions(std::vector<PopupOption> NewOptions)
{
	Options = NewOptions;
	OptionsList->DeleteChildren();
	for (size_t i = 0; i < Options.size(); i++)
	{
		OptionsList->AddChild(
			(new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, (int)i))
			->SetPadding(0.005f)
			->SetBorder(UIBox::BorderType::Rounded, 0.2f)
			->AddChild((new UIText(0.4f, 1 - EditorUI::UIColors[2], Options[i].Name, EditorUI::Text))
				->SetPadding(0.005f)));
	}
}

bool EditorPopup::IsHoveringAnyPopup()
{
	if (!UI::HoveredBox)
	{
		return false;
	}

	for (UICanvas* i : Graphics::UIToRender)
	{
		EditorPopup* Popup = dynamic_cast<EditorPopup*>(i);
		if (!Popup)
		{
			continue;
		}
		if (UI::HoveredBox->IsChildOf(Popup->RootBox))
		{
			return true;
		}
	}
	return false;
}

EditorPopup::EditorPopup(Vector2 Position, Vector2 Scale, std::string Name)
{
	Position = Position - Scale / 2;
	RootBox = new UIBackground(UIBox::Orientation::Vertical, Position, 0.5f);

	TitleBackground = new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 0.75, Vector2(Scale.X, 0.05f));
	OptionsList = new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[1], Vector2(Scale.X, 0.05f));
	PopupBackground = new UIBackground(UIBox::Orientation::Vertical, 0, EditorUI::UIColors[0]);
	RootBox
		->AddChild(TitleBackground
			->SetPadding(1, 0, 1, 1)
			->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
			->SetVerticalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(0.5f, EditorUI::UIColors[2], Name, EditorUI::Text))
				->SetPadding(0, 0, 0.01f, 0.01f)))
		->AddChild(PopupBackground
			->SetPadding(0, 0, 1, 1)
			->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
			->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f)
			->SetMinSize(Scale - Vector2(0, 0.1f)))
		->AddChild(OptionsList
			->SetPadding(0, 1, 1, 1)
			->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
			->SetHorizontalAlign(UIBox::Align::Reverse));

	RootBox->HasMouseCollision = true;
	TitleBackground->HasMouseCollision = true;
}

void EditorPopup::TickPopup()
{
	if (TitleBackground == UI::HoveredBox && Input::IsLMBClicked && !DraggedPopup)
	{
		DraggedPopup = this;
		DraggedOffset = Input::MouseLocation - RootBox->GetPosition();
	}

	if (DraggedPopup == this)
	{
		Vector2 NewPosition = Input::MouseLocation - DraggedOffset;
		NewPosition = NewPosition.Clamp(-1, 1 - RootBox->GetUsedSize());
		RootBox->SetPosition(NewPosition);
	}
	if (!Input::IsLMBDown && DraggedPopup)
	{
		DraggedPopup = nullptr;
	}
}

EditorPopup::~EditorPopup()
{
	delete RootBox;
}

void EditorPopup::HandlePopupButtons(int Index)
{
	if (Index >= 0 && Index < Options.size())
	{
		if (Options.at(Index).OnClicked)
		{
			Options.at(Index).OnClicked();
		}
		if (Options.at(Index).Close)
		{
			delete this;
		}
	}
}
#endif