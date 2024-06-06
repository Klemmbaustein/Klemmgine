#if EDITOR
#pragma once
#include <UI/UICanvas.h>

class EditorDropdown : public UICanvas
{
	UIBox* Root = nullptr;
public:
	struct DropdownItem
	{
		std::string Title;
		void (*OnPressed)() = nullptr;
		bool Separator = false;
	};

	EditorDropdown(std::vector<DropdownItem> Menu, Vector2 Position);
	~EditorDropdown();

	void Tick() override;

	void OnButtonClicked(int Index) override;

	std::vector<DropdownItem> Options;

};
#endif