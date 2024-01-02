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
public:
	size_t ObjectSize = 0;
	size_t ListIterator = 0;
	UIScrollBox* ObjectListBox;
	bool RecalculateObjects = false;

	ObjectList(EditorPanel* Parent);
	
	void Tick() override;
	void OnButtonClicked(int Index) override;
	void OnResized() override;

	void GenerateObjectListSection(std::vector<EditorUI::ObjectListItem> Section, float Depth);
};
#endif