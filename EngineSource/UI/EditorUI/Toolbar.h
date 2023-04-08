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
		};
		std::vector<Button> Buttons;
	};
protected:
	void GenerateButtons();
	std::vector<ButtonCategory> Buttons;
public:
	Toolbar(Vector3* Colors, Vector2 Position, Vector2 Scale);
	void UpdateLayout() override;
	void Tick() override;

	void OnButtonClicked(int Index) override;

	void RegisterNewButtonCategory(ButtonCategory NewButtons);
};
#endif