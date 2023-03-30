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
	std::map<std::string, size_t> ExtensionTabIDs =
	{
		{"jsm", 1},
		{"jsmat", 2},
		{"jsmtmp", 3}

	};

	bool RMBDown = false;
	struct FileEntry
	{
		std::string Name;
		bool IsDirectory;
	};
	static std::vector<FileEntry> CurrentFiles;
	int IsDraggingButton = 0;
	static std::vector<UIButton*> Buttons;
	int DraggedButton = 0;
	UIBox* ContentBox = nullptr;
public:
	void ScanForAssets();
	std::vector<EditorClassesItem> CPPClasses;
	std::vector<size_t> CPPPath;
	static void DeleteFile();
	int SelectedTab = 0;
	std::vector<std::string> Tabs =
	{
		"Assets",
		"Classes"
	};
	std::vector<EditorClassesItem> GetEditorUIClasses();
	std::vector<EditorClassesItem> GetContentsOfCurrentCPPFolder();
	std::string GetCurrentCPPPathString();

	UIScrollBox* BrowserScrollBox;
	ItemBrowser(Vector3* Colors, Vector2 Position, Vector2 Scale);
	void Save() override;
	void Load(std::string File) override;
	void UpdateLayout() override;
	void Tick() override;

	void OnButtonDragged(int Index) override;
	void OnButtonClicked(int Index) override;
};
#endif