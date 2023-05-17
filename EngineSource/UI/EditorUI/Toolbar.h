#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
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
	void GenerateButtons();
	std::vector<ButtonCategory> Buttons;
public:

	void RemoveButton(std::string Name);
	void SetButtonVisibility(std::string Name, bool IsVisible);

	Toolbar(Vector3* Colors, Vector2 Position, Vector2 Scale);
	void UpdateLayout() override;
	void Tick() override;

	void OnButtonClicked(int Index) override;

	void RegisterNewButtonCategory(ButtonCategory NewButtons);
};
#endif