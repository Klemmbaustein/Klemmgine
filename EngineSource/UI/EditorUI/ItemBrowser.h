#if EDITOR
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>
#include <Objects/WorldObject.h>
#include <UI/EditorUI/EditorUI.h>


/**
* @brief
* Editor panel that displays a list of items.
* 
* Used for class and asset view panels.
* 
* @ingroup Editor
*/
class ItemBrowser : public EditorPanel
{
	UITextField* PathField = nullptr;
	bool RMBDown = false;
	UIBackground* SeperatorLine = nullptr;
	UIBox* TopBox = nullptr;
protected:
	static ItemBrowser* DropdownBrowser;
public:
	UIBackground* DraggedButton = nullptr;

	/**
	* @brief
	* 
	* The path displayed in the top section of the item browser.
	*/
	std::string Path;

	struct BrowserItem
	{
		unsigned int Texture = 0;
		std::string Name;
		std::string Path;
		Vector3 Color;
		uint32_t TypeID = 0;
		bool Selected = false;
		bool Deleteable = true;
		bool Renameable = true;
		bool Openable = true;
	};

	std::vector<EditorUI::DropdownItem> DefaultDropdown;
	std::vector<EditorUI::DropdownItem> ContextOptions;

	std::string EmptyText = "No items";
	/**
	* @brief
	* Gets displayed items.
	* 
	* @return
	* A list of items that should be displayed in the item browser.
	*/
	virtual std::vector<BrowserItem> GetBrowserContents() = 0;

	/**
	* @brief
	* Called when an item in the browser has been clicked.
	* 
	* @param Item
	* The clicked item.
	*/
	virtual void OnItemClicked(BrowserItem Item) = 0;

	virtual void GoBack() = 0;

	UIScrollBox* BrowserScrollBox;
	ItemBrowser(EditorPanel* Parent, std::string Name);
	void OnResized() override;
	void Tick() override;
	virtual void DeleteItem(BrowserItem Item);

	void OnPathChanged();
	void GenerateTopBox();
	void GenerateAssetList();

	virtual void OnButtonDragged(int Index) override;
	virtual void OnButtonClicked(int Index) override;
private:
	std::vector<UIButton*> Buttons;
	std::vector<BrowserItem> LoadedItems;
};
#endif