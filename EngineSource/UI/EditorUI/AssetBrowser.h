#if EDITOR
#pragma once
#include <UI/EditorUI/ItemBrowser.h>

/**
* @brief
* Item browser for browsing files in the Content/ directory in the editor.
* 
* @ingroup Editor
*/
class AssetBrowser : public ItemBrowser
{
public:
	AssetBrowser(EditorPanel* Parent);

	virtual std::vector<BrowserItem> GetBrowserContents() override;
	virtual void OnItemClicked(BrowserItem Item) override;
	virtual void GoBack() override;

	virtual void OnItemDropped(DroppedItem From, BrowserItem To);

	virtual void DeleteItem(BrowserItem Item) override;
	static void UpdateAll();
};
#endif