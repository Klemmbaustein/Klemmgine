#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>
#include <UI/EditorUI/EditorUI.h>

class ObjectList : public EditorPanel
{
public:
	size_t ObjectSize = 0;
	UIBox* HeaderBox;
	UIScrollBox* ObjectListBox;
	bool RecalculateObjects = false;

	ObjectList(Vector3* Colors, Vector2 Position, Vector2 Scale);
	
	void Tick() override;
	void OnButtonClicked(int Index) override;
	void UpdateLayout() override;

	void Save() override;
	void Load(std::string File) override;

	void GenerateObjectListSection(std::vector<EditorUI::ObjectListItem> Section, float Depth);
};
#endif