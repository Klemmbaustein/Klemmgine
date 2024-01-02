#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>

/**
* @brief
* Toolbar panel in the editor.
* 
* @ingroup Editor
*/
class Toolbar : public EditorPanel
{
public:
	struct ButtonCategory
	{
		std::string Name;
		struct Button
		{
			std::string Name;
			unsigned int Texture;
			void (*OnPressed)();
			bool IsVisible = true;
		};
		std::vector<Button> Buttons;
	};

	static Toolbar* ToolbarInstance;

protected:
	UIBox* ButtonsBox = nullptr;
	void GenerateButtons();
	std::vector<ButtonCategory> Buttons;
public:

	void RemoveButton(std::string Name);
	void SetButtonVisibility(std::string Name, bool IsVisible);

	Toolbar(EditorPanel* Parent);
	void OnResized() override;
	void Tick() override;

	void OnButtonClicked(int Index) override;

	void RegisterNewButtonCategory(ButtonCategory NewButtons);
};
#endif