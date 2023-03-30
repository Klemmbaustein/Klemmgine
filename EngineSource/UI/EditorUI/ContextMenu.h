#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <Engine/TypeEnun.h>
#include <UI/UIfwd.h>
#include <Objects/WorldObject.h>

class WorldObject;

class ContextMenu : public EditorPanel
{
public:
	UIScrollBox* BackgroundBox;
	std::vector<Objects::Property> Properties;
	ContextMenu(Vector3* Colors, Vector2 Position, Vector2 Scale);
	struct ContextMenuSection
	{
		void* Variable;
		Type::TypeEnum Type;
		std::string Name;
		bool Normalized;
		ContextMenuSection(void* Variable, Type::TypeEnum Type, std::string Name, bool Normalized = false)
		{
			this->Variable = Variable;
			this->Type = Type;
			this->Name = Name;
			this->Normalized = Normalized;
		}
	};

	std::vector<ContextMenuSection> ContextSettings;
	std::vector<std::string> ContextCategories;
	std::vector<UIBox*> ContextButtons;

	UITextField* GenerateTextField(std::string Content);
	void GenerateSection(std::vector<ContextMenuSection> Section, std::string Name, WorldObject* ContextObject, unsigned int Index);

	void Tick() override;
	void OnButtonClicked(int Index) override;
	void UpdateLayout() override;

	void Load(std::string File) override;
	void Save() override;
};
#endif