#if EDITOR
#include "EditorDropdown.h"
#include <UI/UIBackground.h>
#include <UI/UIButton.h>
#include "EditorUI.h"
#include <Engine/Input.h>

static EditorDropdown* CurrentDropdown = nullptr;

EditorDropdown::EditorDropdown(std::vector<DropdownItem> Menu, Vector2 Position)
{
	if (CurrentDropdown)
	{
		delete CurrentDropdown;
	}
	CurrentDropdown = this;

	Options = Menu;
	Root = new UIBox(UIBox::Orientation::Vertical, Position);
	auto Background = new UIBackground(UIBox::Orientation::Vertical, 0, Vector3::Lerp(EditorUI::UIColors[0], EditorUI::UIColors[2], 0.5f), 0);
	Root->AddChild(Background
		->SetMinSize(Vector2(0.15f, 0)));

	for (size_t i = 0; i < Menu.size(); i++)
	{
		float PaddingSize = 2.0f / Graphics::WindowResolution.Y;
		bool Upper = i == 0;
		bool Lower = i == Menu.size() - 1 || Menu[i].Separator;

		float Horizontal = 2.0f / Graphics::WindowResolution.X;

		Background->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 1.5f, this, (int)i))
			->SetTryFill(true)
			->SetPadding(PaddingSize * (float)Upper, PaddingSize * (float)Lower, Horizontal, Horizontal)
			->SetMinSize(Vector2(0.15f, 0))
			->AddChild((new UIText(0.45f, EditorUI::UIColors[2], Menu[i].Title, EditorUI::Text))
				->SetPadding(0.005f)));
	}

	Root->UpdateSelfAndChildren();
	Root->SetPosition((Root->GetPosition() - Vector2(0, Root->GetUsedSize().Y * 4))
		.Clamp(Vector2(-1, -1), Vector2(1 - Root->GetUsedSize().X, 1 - Root->GetUsedSize().Y)));

}

EditorDropdown::~EditorDropdown()
{
	delete Root;
	if (CurrentDropdown == this)
	{
		CurrentDropdown = nullptr;
	}
}

void EditorDropdown::Tick()
{
	if (Input::IsLMBDown && !Root->IsHovered())
	{
		delete this;
	}
}

void EditorDropdown::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		if (Options[Index].OnPressed)
		{
			Options[Index].OnPressed();
		}
		delete this;
	}
}
#endif