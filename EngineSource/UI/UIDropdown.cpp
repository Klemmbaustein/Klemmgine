#include "UIDropdown.h"
#include <UI/UIText.h>
#include <Engine/Log.h>
#include <Engine/Application.h>
#include <Engine/Input.h>

UIDropdown::UIDropdown(Vector2 Position,
	float Size,
	Vector3 Color,
	Vector3 TextColor,
	std::vector<Option> Options,
	int Index,
	UICanvas* Parent,
	TextRenderer* Renderer)
	:
	UIButton(true,
		Position,
		Color,
		Parent,
		Index)
{
	this->Renderer = Renderer;
	this->Options = Options;
	this->Size = Size;
	if (!this->Options.size())
	{
		this->Options.push_back(Option());
	}
	SelectedOption = Options.at(0);
	SelectedText = new UIText(TextSize, TextColor, this->Options.at(0).Name, Renderer);
	SelectedText->SetPadding(TextPadding);
	AddChild(SelectedText);
	SetMinSize(Vector2(Size, 0));

	OptionsBox = new UIBox(false, Position + Vector2(0, -1));
	OptionsBox->SetMinSize(Vector2(0, 1));
	OptionsBox->SetAlign(UIBox::Align::Reverse);
	OptionsBox->IsVisible = false;
	GenerateOptions();
}

UIDropdown* UIDropdown::SetTextSize(float Size, float Padding)
{
	SelectedText->SetTextSize(Size);
	SelectedText->SetPadding(Padding);
	this->TextSize = Size;
	this->TextPadding = Padding;
	return this;
}

UIDropdown* UIDropdown::SetDropdownColor(Vector3 NewColor, Vector3 TextColor)
{
	if (NewColor != DropdownColor || TextColor != DropdownTextColor)
	{
		for (UIButton* i : DropdownButtons)
		{
			i->SetColor(NewColor);
		}
		for (UIText* i : DropdownTexts)
		{
			i->SetColor(TextColor);
		}
		DropdownColor = NewColor;
		DropdownTextColor = TextColor;
	}
	return this;
}

void UIDropdown::GenerateOptions()
{
	OptionsBox->DeleteChildren();
	for (size_t i = 0; i < Options.size(); i++)
	{
		UIButton* NewButton = new UIButton(true, 0, Vector3::Lerp(DropdownColor, Color, (i == SelectedIndex) ? 0.5f : 0), nullptr, (int)i);
		NewButton->SetPadding(0);
		NewButton->SetMinSize(Vector2(Size, 0));
		NewButton->ParentOverride = this;

		UIText* NewText = new UIText(TextSize, DropdownTextColor, Options[i].Name, Renderer);
		NewText->SetPadding(TextPadding);
		NewButton->AddChild(NewText);

		OptionsBox->AddChild(NewButton);
		DropdownButtons.push_back(NewButton);
	}
}

UIDropdown* UIDropdown::SelectOption(size_t Index)
{
	SelectedIndex = Index;
	SelectedOption = Options.at(Index);
	SelectedText->SetText(SelectedOption.Name);

	if (ParentUI)
	{
		Application::ButtonEvents.insert(ButtonEvent(nullptr, ParentUI, ButtonIndex));
	}
	if (Parent || ParentOverride)
	{
		Application::ButtonEvents.insert(ButtonEvent(ParentOverride ? ParentOverride : Parent, nullptr, ButtonIndex));
	}
	GenerateOptions();
	return this;
}

void UIDropdown::Tick()
{
	UIButton::Tick();
	if (Input::IsLMBDown
		&& OptionsBox->IsVisible
		&& (!UI::HoveredBox || !(UI::HoveredBox == this || UI::HoveredBox->IsChildOf(OptionsBox))))
	{
		OptionsBox->IsVisible = false;
	}

	OptionsBox->SetPosition(OffsetPosition + Vector2(0, -1));
}

void UIDropdown::OnClicked()
{
	OptionsBox->IsVisible = !OptionsBox->IsVisible;
}

void UIDropdown::OnChildClicked(int Index)
{
	SelectOption((size_t)Index);
	OptionsBox->IsVisible = false;
}
