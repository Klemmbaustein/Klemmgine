#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <Engine/TypeEnun.h>
#include <UI/UIfwd.h>
#include <Objects/SceneObject.h>

class SceneObject;

/**
* @brief
* EditorPanel displaying information about a selected object or the currently loaded scene.
* 
* @ingroup Editor
*/
class ContextMenu : public EditorPanel
{
	bool IsObject = false;
public:
	UIScrollBox* BackgroundBox;
	std::vector<SceneObject::Property> Properties;
	ContextMenu(EditorPanel* Parent, bool IsScene);
	struct ContextMenuSection
	{
		void* Variable;
		NativeType::NativeType NativeType;
		std::string Name;
		bool Normalized;
		ContextMenuSection(void* Variable, NativeType::NativeType NativeType, std::string Name, bool Normalized = false)
		{
			this->Variable = Variable;
			this->NativeType = NativeType;
			this->Name = Name;
			this->Normalized = Normalized;
		}
	};

	std::vector<ContextMenuSection> ContextSettings;
	std::vector<std::string> ContextCategories;
	std::vector<UIBox*> ContextButtons;

	UITextField* GenerateTextField(std::string Content, int Index);
	void GenerateSection(std::vector<ContextMenuSection> Section, std::string Name, SceneObject* ContextObject, unsigned int Index);
	void GenerateSectionElement(ContextMenuSection Element, SceneObject* ContextObject, std::string Name);

	void Tick() override;
	void OnButtonClicked(int Index) override;
	void OnResized() override;
private:
	void GenerateCSharpProperty(const ContextMenu::ContextMenuSection& Element, SceneObject* ContextObject);
};
#endif