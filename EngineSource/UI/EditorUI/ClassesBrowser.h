#if EDITOR
#pragma once
#include <UI/EditorUI/ItemBrowser.h>

/**
* @brief
* Item browser for browsing C++ or C# classes in the editor.
*
* @ingroup Editor
*/
class ClassesBrowser : public ItemBrowser
{
public:
	struct EditorClassesItem
	{
		std::string Name;
		ObjectDescription Object = ObjectDescription("", 0);
		std::vector<EditorClassesItem> SubItems;
		bool IsFolder = false;
	};

	static std::vector<EditorClassesItem> CPPClasses;
	static std::vector<size_t> CPPPath;
	std::vector<EditorClassesItem> GetEditorUIClasses();
	std::vector<EditorClassesItem> GetContentsOfCurrentCPPFolder();
	std::string GetCurrentCPPPathString();

	ClassesBrowser(EditorPanel* Parent);

	virtual std::vector<BrowserItem> GetBrowserContents() override;
	virtual void OnItemClicked(BrowserItem Item) override;
	virtual void GoBack() override;
};
#endif