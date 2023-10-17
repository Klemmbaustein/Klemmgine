#pragma once
#include <UI/UIButton.h>
#include <UI/UIfwd.h>

class UIDropdown : public UIButton
{
	TextRenderer* Renderer;
	UIText* SelectedText = nullptr;
	UIBox* OptionsBox = nullptr;
	float Size = 0.0f;
	float TextSize = 0.4f, TextPadding = 0.02f;
	std::vector<UIButton*> DropdownButtons;
	std::vector<UIText*> DropdownTexts;
	Vector3 DropdownColor = 1;
	Vector3 DropdownTextColor = 0;
public:
	size_t SelectedIndex = 0;
	struct Option
	{
		std::string Name;
	};

	Option SelectedOption;

	std::vector<Option> Options;
	void GenerateOptions();
	
	UIDropdown* SelectOption(size_t Index);

	UIDropdown(Vector2 Position, float Size, Vector3 Color, Vector3 TextColor, std::vector<Option> Options, int Index, UICanvas* Parent, TextRenderer* Renderer);
	UIDropdown* SetTextSize(float Size, float Padding);
	UIDropdown* SetDropdownColor(Vector3 NewColor, Vector3 TextColor);

	void Tick() override;
	void OnClicked() override;
	void OnChildClicked(int Index) override;
};