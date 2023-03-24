#include "ItemBrowser.h"
#include <filesystem>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>
#include <Engine/FileUtility.h>
#include <Engine/OS.h>

ItemBrowser::ItemBrowser(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorTab(Colors, Position, Scale, Vector2(0.2, 1), Vector2(0.9, 4))
{
	TabBackground->SetHorizontal(false);
	TabBackground->Align = UIBox::E_REVERSE;
	TabBackground->AddChild((new UIButton(true, 0, Vector3(0.2, 0.7, 0), this, -1))
		->SetBorder(UIBox::E_ROUNDED, 0.3)
		->AddChild((new UIText(0.5, 0, "Import", Editor::CurrentUI->EngineUIText))));
	BrowserScrollBox = new UIScrollBox(false, Scale - Vector2(0, 0.1), 25);
	TabBackground->AddChild(BrowserScrollBox
		->SetPadding(0.01, 0, 0, 0));
	UpdateLayout();
}

void ItemBrowser::Save()
{
}

void ItemBrowser::Load(std::string File)
{
}

void ItemBrowser::UpdateLayout()
{
	const int ITEMS_PER_SLICE = Scale.X * 7 + 1;
	BrowserScrollBox->DeleteChildren();
	std::vector<UIBox*> HorizontalSlices;

	std::vector<std::string> Files;
	for (const auto& entry : std::filesystem::directory_iterator("Content/"))
	{
		Files.push_back(entry.path().string());
	}

	HorizontalSlices.resize(Files.size() / ITEMS_PER_SLICE + 1);
	for (UIBox*& i : HorizontalSlices)
	{
		i = new UIBox(true, 0);
		i->SetPadding(0, 0, 0.02, 0);
		i->Align = UIBox::E_REVERSE;
		BrowserScrollBox->AddChild(i);
	}
	for (size_t i = 0; i < Files.size(); i++)
	{
		auto ext = FileUtil::GetExtension(Files[i]);

		if (std::filesystem::is_directory(Files[i]))
		{
			ext = "dir";
		}
		unsigned int TextureID = 4; // 4 -> X symbol
		if (ItemTextures.contains(ext))
		{
			TextureID = ItemTextures[ext];
		}
		Vector3 Color = Vector3(0.8, 0, 0);
		if (ItemColors.contains(ext))
		{
			Color = ItemColors[ext];
		}

		auto NewBackground = new UIButton(false, 0, UIColors[1], this, i);
		NewBackground->SetMinSize(Vector2(0.14, 0.19));
		NewBackground->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
		NewBackground->SetPadding(0.005 * Graphics::AspectRatio, 0.005 * Graphics::AspectRatio, 0.005, 0.005);
		NewBackground->Align = UIBox::E_REVERSE;
		NewBackground->SetBorder(UIBox::E_ROUNDED, 0.5);

		UIBackground* ItemImage = new UIBackground(true, 0, 1, Vector2(0.12));
		ItemImage->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
		ItemImage->SetUseTexture(true, Editor::CurrentUI->Textures[TextureID]);
		ItemImage->SetPadding(0);
		NewBackground->AddChild((new UIBackground(true, 0, Color, 0.1))
			->SetBorder(UIBox::E_ROUNDED, 0.5)
			->SetSizeMode(UIBox::E_PIXEL_RELATIVE)
			->SetPadding(0.0025 * Graphics::AspectRatio, 0, 0.005, 0.005)
			->AddChild(ItemImage));

		auto ItemText = new UIText(0.375, UIColors[2], FileUtil::GetFileNameWithoutExtensionFromPath(Files[i]), Editor::CurrentUI->EngineUIText);
		ItemText->Wrap = true;
		ItemText->WrapDistance = 0.09;
		ItemText->SetTextWidthOverride(0.0);
		NewBackground->AddChild(ItemText
			->SetPadding(0.0025));

		HorizontalSlices[HorizontalSlices.size() - i / ITEMS_PER_SLICE - 1]->AddChild(NewBackground);
	}
}

void ItemBrowser::Tick()
{
	UpdateTab();
}

void ItemBrowser::OnButtonClicked(int Index)
{
	if (Index == -1)
	{
		std::string file = OS::ShowOpenFileDialog();
		if (std::filesystem::exists(file))
		{
			std::filesystem::copy(file, "Content/" + FileUtil::GetFileNameFromPath(file));
		}
	}
	if (Index >= 0)
	{

	}
}
