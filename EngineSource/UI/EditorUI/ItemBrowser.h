#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>
#include <Objects/WorldObject.h>

struct EditorClassesItem
{
	std::string Name;
	ObjectDescription Object = ObjectDescription("", 0);
	std::vector<EditorClassesItem> SubItems;
	bool IsFolder = false;
};

class ItemBrowser : public EditorPanel
{
	UITextField* PathField = nullptr;
	struct FileEntry
	{
		std::string Name;
		bool IsDirectory;
	};
	static size_t SelectedButton;
	static std::vector<FileEntry> CurrentFiles;
	int IsDraggingButton = 0;
	static std::vector<UIButton*> Buttons;
	int DraggedButton = 0;
	bool RMBDown = false;
	UIBox* ContentBox = nullptr;
public:
	void ScanForAssets();
	static std::vector<EditorClassesItem> CPPClasses;
	static std::vector<size_t> CPPPath;

	int SelectedTab = 0;
	std::vector<std::string> Tabs =
	{
		"Assets",
		"Classes"
	};
	std::vector<EditorClassesItem> GetEditorUIClasses();
	std::vector<EditorClassesItem> GetContentsOfCurrentCPPFolder();
	static std::string GetCurrentCPPPathString();

	UIScrollBox* BrowserScrollBox;
	ItemBrowser(Vector3* Colors, Vector2 Position, Vector2 Scale);
	void UpdateLayout() override;
	void Tick() override;

	void OnButtonDragged(int Index) override;
	void OnButtonClicked(int Index) override;
};
#endif