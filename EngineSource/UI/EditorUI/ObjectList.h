#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>
#include <UI/EditorUI/EditorUI.h>

/**
* @brief
* EditorPanel displaying all objects in the current scene.
* 
* @ingroup Editor
*/
class ObjectList : public EditorPanel
{
	size_t ObjectSize = 0;
	size_t ListIterator = 0;
	UIScrollBox* ObjectListBox = nullptr;
	bool RecalculateObjects = false;

	struct ObjectButton
	{
		UIButton* Button = nullptr;
		int Index = 0;
	};
	std::vector<std::string> ObjectCategories;
	std::set<std::string> CollapsedItems;

public:

	struct ObjectListItem
	{
		ObjectListItem(std::string CategoryName, std::vector<ObjectListItem> Children, bool IsScene, bool IsCollapsed)
		{
			this->Name = CategoryName;
			this->Children = Children;
			this->IsScene = IsScene;
			this->IsCollapsed = IsCollapsed;
		}
		ObjectListItem(SceneObject* Object, int ListIndex)
		{
			this->Object = Object;
			this->ListIndex = ListIndex;
			if (Object)
			{
				Name = Object->Name;
				IsSelected = Object->IsSelected;
			}
		}

		bool IsSelected = false;
		bool IsCollapsed = false;
		std::vector<ObjectListItem> Children;
		std::string Name;
		SceneObject* Object = nullptr;
		int ListIndex = -1;
		bool IsScene = false;
	};

	ObjectList(EditorPanel* Parent);
	
	void Tick() override;
	void OnButtonClicked(int Index) override;
	void OnResized() override;

	void GenerateObjectListSection(std::vector<ObjectListItem> Section, float Depth);
	std::vector<ObjectListItem> GetObjectList();
};
#endif