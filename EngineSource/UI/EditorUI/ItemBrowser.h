#if EDITOR
#pragma once
#include <UI/EditorUI/EditorTab.h>
#include <UI/UIfwd.h>

class ItemBrowser : public EditorTab
{
	std::map<std::string, Vector3> ItemColors
	{
		std::pair("dir", Vector3(0.8, 0.5, 0)),
		std::pair("jsmat", Vector3(0, 0.7, 0)),
		std::pair("jsmtmp", Vector3(0.1, 0.4, 0)),
		std::pair("jsm", Vector3(0, 0, 0.75)),
		std::pair("jscn", Vector3(1.0, 0.4, 0.4)),
		std::pair("png", Vector3(0.3, 0, 1)),
		std::pair("cbm", Vector3(0.7, 0.1, 0.4)),
		std::pair("jspart", Vector3(0.7, 0.4, 0.4)),
		std::pair("wav", Vector3(0.7, 0, 0.4))
	};
	std::map<std::string, unsigned int> ItemTextures
	{
		std::pair("dir", 5),
		std::pair("jsmat", 9),
		std::pair("jsmtmp", 10),
		std::pair("jsm", 11),
		std::pair("jscn", 7),
		std::pair("png", 18),
		std::pair("cbm", 17),
		std::pair("wav", 6)
	};
	struct FileEntry
	{
		std::string Name;
		bool IsDirectory;
	};
	std::vector<FileEntry> CurrentFiles;

public:
	void ScanForAssets();

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