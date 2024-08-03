#if EDITOR
#include "ItemBrowser.h"
#include <UI/UIScrollBox.h>
#include <UI/UIButton.h>
#include <Engine/Log.h>
#include <Engine/Application.h>
#include <Engine/Input.h>
#include <Engine/Utility/FileUtility.h>
#include <UI/EditorUI/Popups/RenameBox.h>
#include <UI/EditorUI/EditorUI.h>

static ItemBrowser::BrowserItem DropdownItem;
static ItemBrowser::BrowserItem DraggedItem;
ItemBrowser* ItemBrowser::DropdownBrowser = nullptr;

ItemBrowser::ItemBrowser(EditorPanel* Parent, std::string Name, std::string ClassName) : EditorPanel(Parent, Name, ClassName)
{
	TopBox = new UIBox(UIBox::Orientation::Horizontal, 0);
	PanelMainBackground->AddChild(TopBox
		->SetMinSize(Vector2(0, 0.1f))
		->SetVerticalAlign(UIBox::Align::Centered)
		->SetTryFill(true));

	SeparatorLine = new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 0.75f, Vector2(0, 6.0f / Graphics::WindowResolution.Y));

	PanelMainBackground->AddChild(SeparatorLine
		->SetPadding(0, 0, 0.005f, 0.005f)
		->SetPaddingSizeMode(UIBox::SizeMode::AspectRelative)
		->SetTryFill(true));

	BrowserScrollBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	BrowserScrollBox->SetMinSize(PanelMainBackground->GetMinSize() - Vector2(0.004f / Graphics::AspectRatio, TabList->GetMinSize().Y - 0.1f));
	PanelMainBackground->AddChild(BrowserScrollBox);
}

void ItemBrowser::OnResized()
{
	SeparatorLine->SetColor(EditorUI::UIColors[0] * 0.75);
	GenerateAssetList();
	GenerateTopBox();
}

void ItemBrowser::Tick()
{
	TickPanel();
	if (!PanelMainBackground->IsVisible)
	{
		return;
	}

	if (DraggedButton)
	{
		DraggedButton->SetPosition(Input::MouseLocation);
		if (!Input::IsLMBDown)
		{
			for (UICanvas* i : Graphics::UIToRender)
			{
				EditorPanel* Panel = dynamic_cast<EditorPanel*>(i);
				if (Panel 
					&& UI::HoveredBox 
					&& (UI::HoveredBox == Panel->PanelMainBackground 
						|| UI::HoveredBox->IsChildOf(Panel->PanelMainBackground)))
				{
					DroppedItem i;
					i.Path = DraggedItem.Path;
					i.TypeID = DraggedItem.TypeID;
					i.From = this;
					Panel->OnItemDropped(i);
				}
			}
			delete DraggedButton;
			DraggedButton = nullptr;
		}
	}

	BrowserScrollBox->SetMinSize(PanelMainBackground->GetMinSize() - Vector2(0.004f / Graphics::AspectRatio, 0.1f + 0.01f));
	BrowserScrollBox->SetMaxSize(BrowserScrollBox->GetMinSize());

	if (Input::IsLMBDown && !(dynamic_cast<UIButton*>(UI::HoveredBox) && UI::HoveredBox->IsChildOf(PanelMainBackground)))
	{
		bool Changed = false;
		for (auto& i : LoadedItems)
		{
			if (i.Selected)
			{
				Changed = true;
			}
			i.Selected = false;
		}
		if (Changed)
		{
			GenerateAssetList();
		}
	}

	if (Input::IsRMBDown && !RMBDown && UI::HoveredBox == BrowserScrollBox)
	{
		new EditorDropdown(DefaultDropdown, Input::MouseLocation);
	}
	else if (Input::IsRMBDown && !RMBDown && dynamic_cast<UIButton*>(UI::HoveredBox) && UI::HoveredBox->IsChildOf(PanelMainBackground))
	{
		size_t ButtonIndex = 0;
		for (ButtonIndex = 0; ButtonIndex < Buttons.size(); ButtonIndex++)
		{
			if (Buttons[ButtonIndex] == UI::HoveredBox)
			{
				break;
			}
		}

		if (ButtonIndex == Buttons.size())
		{
			RMBDown = Input::IsRMBDown;
			return;
		}

		std::vector<EditorDropdown::DropdownItem> Items = ContextOptions;

		DropdownItem = LoadedItems[ButtonIndex];
		DropdownBrowser = this;
		if (DropdownItem.Openable)
		{
			Items.push_back(EditorDropdown::DropdownItem("Open", []() { DropdownBrowser->OnItemClicked(DropdownItem); }));
		}
		if (DropdownItem.Renameable)
		{
			Items.push_back(EditorDropdown::DropdownItem("Rename", []() { new RenameBox(DropdownItem.Path); }));
		}
		if (DropdownItem.CanCopy)
		{
			Items.push_back(EditorDropdown::DropdownItem("Copy", []()
				{
					std::filesystem::copy(
						DropdownItem.Path,
						FileUtil::GetFilePathWithoutExtension(DropdownItem.Path) + "_Copy." + FileUtil::GetExtension(DropdownItem.Path)
					); 
					DropdownBrowser->OnPathChanged();
				}));
		}
		if (DropdownItem.Deleteable)
		{
			Items.push_back(EditorDropdown::DropdownItem("Delete", []()
				{ 
					DropdownBrowser->DeleteItem(DropdownItem);
					DropdownBrowser->OnPathChanged();
				}));
		}

		if (!Items.empty())
		{
			new EditorDropdown(Items, Input::MouseLocation);
		}
	}

	RMBDown = Input::IsRMBDown;
}

void ItemBrowser::DeleteItem(BrowserItem Item)
{
}

void ItemBrowser::OnPathChanged()
{
	LoadedItems = GetBrowserContents();
	GenerateAssetList();
	GenerateTopBox();
}

void ItemBrowser::OnItemDropped(DroppedItem Item)
{
	if (Item.From != this)
	{
		return;
	}

	if (UI::HoveredBox == UpButton)
	{
		std::string NewPath = Path;
		if (NewPath.substr(NewPath.size() - 1) == "/")
		{
			NewPath.pop_back();
		}
		size_t LastSlash = NewPath.find_last_of("/");
		if (LastSlash == std::string::npos)
		{
			return;
		}
		NewPath = NewPath.substr(0, LastSlash + 1);
		BrowserItem i;
		i.Path = NewPath;
		OnItemDropped(Item, i);
	}

	for (size_t i = 0; i < Buttons.size(); i++)
	{
		if (UI::HoveredBox == Buttons[i])
		{
			OnItemDropped(Item, LoadedItems[i]);
		}
	}
}

void ItemBrowser::OnItemDropped(DroppedItem From, BrowserItem To)
{
}

void ItemBrowser::GenerateTopBox()
{
	TopBox->DeleteChildren();
	UpButton = new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, -1);
	TopBox->AddChild(UpButton
		->SetUseTexture(true, EditorUI::Textures[8])
		->SetPadding(0.01f)
		->SetPaddingSizeMode(UIBox::SizeMode::AspectRelative)
		->SetMinSize(0.05f)
		->SetSizeMode(UIBox::SizeMode::AspectRelative));

	TopBox->AddChild((new UIText(0.45f, EditorUI::UIColors[2], Path, EditorUI::Text))
		->SetWrapEnabled(true, Scale.X - 0.1f / Graphics::AspectRatio, UIBox::SizeMode::ScreenRelative)
		->SetPadding(0.005f));
}

void ItemBrowser::GenerateAssetList()
{
	if (BrowserScrollBox->GetMinSize().X == 0)
	{
		return;
	}

	BrowserScrollBox->DeleteChildren();

	std::vector<UIBox*> HorizontalBoxes;

	int SlotsPerRow = (size_t)((Scale.X - 0.04f) / (0.14f / Graphics::AspectRatio));
	if (SlotsPerRow <= 0)
	{
		return;
	}
	for (int i = 0; i < (int)LoadedItems.size() / SlotsPerRow + 1; i++)
	{
		UIBox* New = (new UIBox(UIBox::Orientation::Horizontal, 0))
			;
		HorizontalBoxes.push_back(New);
		BrowserScrollBox->AddChild(New);
	}
	Buttons.clear();
	int Index = 0;
	for (const auto& Item : LoadedItems)
	{
		UIButton* Button = new UIButton(UIBox::Orientation::Vertical, 0, Item.Selected ? Vector3(0.3f, 0.3f, 0.3f) : EditorUI::UIColors[1], this, Index);
		Buttons.push_back(Button);
		HorizontalBoxes[Index / SlotsPerRow]->AddChild(Button
			->SetCanBeDragged(true)
			->SetBorder(UIBox::BorderType::Rounded, 0.4f)
			->SetMinSize(Vector2(0, 0.2f))
			->SetPadding(0.01f, 0.0f, 0.01f, 0.0f)
			->SetPaddingSizeMode(UIBox::SizeMode::AspectRelative)
			->SetSizeMode(UIBox::SizeMode::AspectRelative)
			->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, Item.Color, 0.125f))
				->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, 1.0f, 0.125f))
					->SetUseTexture(true, Item.Texture)
					->SetSizeMode(UIBox::SizeMode::AspectRelative))
				->SetBorder(UIBox::BorderType::Rounded, 0.4f)
				->SetPadding(0.005f)
				->SetSizeMode(UIBox::SizeMode::AspectRelative)
				->SetPaddingSizeMode(UIBox::SizeMode::AspectRelative))
			->AddChild((new UIText(0.38f, EditorUI::UIColors[2], Item.Name, EditorUI::Text))
				->SetWrapEnabled(true, 0.12f, UIBox::SizeMode::AspectRelative)
				->SetPadding(0, 0, 0.005f, 0.005f)
				->SetPaddingSizeMode(UIBox::SizeMode::AspectRelative)));
		Index++;
	}

	if (LoadedItems.empty())
	{
		HorizontalBoxes[0]->AddChild((new UIText(0.45f, EditorUI::UIColors[2], EmptyText, EditorUI::Text))
			->SetWrapEnabled(true, Scale.X - 0.1f, UIBox::SizeMode::ScreenRelative)
			->SetPadding(0.01f));
	}
}

void ItemBrowser::OnButtonDragged(int Index)
{
	HandlePanelDrag(Index);
	if (DraggedButton)
	{
		return;
	}
	if (Index < 0)
	{
		return;
	}

	auto& Item = LoadedItems[Index];
	DraggedButton = new UIBackground(UIBox::Orientation::Horizontal, Input::MouseLocation, Item.Color, 0.125f);
	DraggedButton
		->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, 1.0f, 0.125f))
			->SetUseTexture(true, Item.Texture)
			->SetSizeMode(UIBox::SizeMode::AspectRelative))
		->SetBorder(UIBox::BorderType::Rounded, 0.4f)
		->SetSizeMode(UIBox::SizeMode::AspectRelative);

	DraggedItem = Item;
}

void ItemBrowser::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		if (LoadedItems[Index].Selected && LoadedItems[Index].Openable)
		{
			OnItemClicked(LoadedItems[Index]);
		}
		else
		{
			for (auto& i : LoadedItems)
			{
				i.Selected = false;
			}
			LoadedItems[Index].Selected = true;
		}
		GenerateAssetList();
		GenerateTopBox();
	}
	if (Index == -1)
	{
		GoBack();
		OnPathChanged();
	}
	HandlePanelButtons(Index);
}
#endif
