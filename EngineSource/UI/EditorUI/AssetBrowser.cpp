#if EDITOR
#include "AssetBrowser.h"
#include <filesystem>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Utility/StringUtility.h>
#include <UI/EditorUI/EditorDropdown.h>
#include <Engine/Log.h>
#include <Engine/Importers/ModelConverter.h>
#include <Engine/Importers/Importer.h>
#include <Engine/OS.h>
#include <UI/EditorUI/Tabs/MaterialTab.h>
#include <Engine/Subsystem/Scene.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/EditorUI/Tabs/MeshTab.h>
#include <UI/EditorUI/Tabs/ParticleEditorTab.h>
#include <Engine/File/Assets.h>

static std::string* CurrentPath = nullptr;

std::vector<AssetBrowser::BrowserItem> AssetBrowser::GetBrowserContents()
{
	Assets::ScanForAssets();
	std::vector<BrowserItem> Items;

	for (const auto& File : std::filesystem::directory_iterator(Path))
	{
		if (!File.is_directory())
		{
			continue;
		}
		BrowserItem Item;
		Item.Name = FileUtil::GetFileNameWithoutExtensionFromPath(StrUtil::UnicodeToAscii(File.path().u8string()));
		std::string Ext = FileUtil::GetExtension(StrUtil::UnicodeToAscii(File.path().u8string()));
		Item.Texture =  EditorUI::Textures[5];
		Item.Color = EditorUI::ItemColors["dir"];
		Item.Path = StrUtil::UnicodeToAscii(File.path().u8string());
		Item.CanCopy = false;
		Items.push_back(Item);
	}

	for (const auto& File : std::filesystem::directory_iterator(Path))
	{
		if (File.is_directory())
		{
			continue;
		}
		BrowserItem Item;
		Item.Name = FileUtil::GetFileNameWithoutExtensionFromPath(StrUtil::UnicodeToAscii(File.path().u8string()));
		std::string Ext = FileUtil::GetExtension(StrUtil::UnicodeToAscii(File.path().u8string()));
		Item.Texture = EditorUI::Textures[EditorUI::ItemTextures[Ext]];
		Item.Color = EditorUI::ItemColors[Ext];
		Item.Path = StrUtil::UnicodeToAscii(File.path().u8string());
		Item.CanCopy = true;
		Items.push_back(Item);
	}

	return Items;
}

void AssetBrowser::OnItemClicked(BrowserItem Item)
{
	if (std::filesystem::is_directory(Item.Path))
	{
#if _WIN32
		StrUtil::ReplaceChar(Item.Path, '\\', "/");
#endif
		Path = Item.Path + "/";
		CurrentPath = &Path;
		OnPathChanged();
		return;
	}
	std::string Extension = FileUtil::GetExtension(Item.Path);

	if (Extension == "jsmat")
	{
		auto NewTab = new MaterialTab(nullptr, Item.Path);
		Viewport::ViewportInstance->AddPanelTab(NewTab);
		NewTab->Load(Item.Path);
	}

	if (Extension == "jsm")
	{
		auto NewTab = new MeshTab(nullptr, Item.Path);
		Viewport::ViewportInstance->AddPanelTab(NewTab);
		NewTab->Load(Item.Path);
	}

	if (Extension == "jspart")
	{
		auto NewTab = new ParticleEditorTab(nullptr, Item.Path);
		Viewport::ViewportInstance->AddPanelTab(NewTab);
	}

	if (Extension == "jscn")
	{
		EditorUI::OpenScene(Item.Path);
	}

	if (Extension == "png" || Extension == "wav")
	{
		OS::OpenFile(Item.Path);
	}
}

void AssetBrowser::GoBack()
{
	if (Path[Path.size() - 1] == '/')
	{
		Path.pop_back();
	}
	Path = Path.substr(0, Path.find_last_of("/"));
	Path.append("/");
	CurrentPath = &Path;
}

void AssetBrowser::OnItemDropped(DroppedItem From, BrowserItem To)
{
	if (!std::filesystem::exists(From.Path))
	{
		return;
	}

	if (From.Path == To.Path)
	{
		return;
	}

	if (std::filesystem::is_directory(To.Path))
	{
		std::filesystem::copy(From.Path,
			To.Path + "/" + FileUtil::GetFileNameFromPath(From.Path),
			std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
		std::filesystem::remove_all(From.Path);
		OnPathChanged();
	}
}

void AssetBrowser::DeleteItem(BrowserItem Item)
{
	std::filesystem::remove_all(Item.Path);
}

void AssetBrowser::UpdateAll()
{
	for (UICanvas* c : Graphics::UIToRender)
	{
		if (dynamic_cast<AssetBrowser*>(c))
		{
			static_cast<AssetBrowser*>(c)->OnPathChanged();
		}
	}

}

AssetBrowser::AssetBrowser(EditorPanel* Parent) : ItemBrowser(Parent, "Assets", "file_browser")
{
	DefaultDropdown =
	{
		EditorDropdown::DropdownItem("Import file", []()
			{
			std::string file = OS::ShowOpenFileDialog();
			if (std::filesystem::exists(file))
			{
				if (EditorUI::ModelFileExtensions.contains(FileUtil::GetExtension(file)))
				{
					ModelImporter::Import(file, *CurrentPath);
				}
				else
				{
					Importer::Import(file, *CurrentPath);
				}
			}
			UpdateAll();
			}, true),

		EditorDropdown::DropdownItem("New folder", []() 
			{ 
				std::filesystem::create_directories(*CurrentPath + "/Folder");
				UpdateAll();
			}),
		EditorDropdown::DropdownItem("New material", []()
			{
				EditorUI::CreateFile(*CurrentPath, "Material", "jsmat");
				UpdateAll();
			}),
		EditorDropdown::DropdownItem("New scene", []()
			{
				EditorUI::CreateFile(*CurrentPath, "Scene", "jscn");
				UpdateAll();
			}),
		EditorDropdown::DropdownItem("New particle", []()
			{
				EditorUI::CreateFile(*CurrentPath, "Particle", "jspart");
				UpdateAll();
			}, true),
		EditorDropdown::DropdownItem("Open in file explorer", []()
			{
#if _WIN32
				std::string Path = *CurrentPath;
				StrUtil::ReplaceChar(Path, '/', "\\");
				system(("explorer.exe \"" + Path + "\"").c_str());
#else
				system(("xdg-open \"" + *CurrentPath + "\"").c_str());
#endif
			}),
	};

	EmptyText = "Right-click to create new assets";
	Path = "Content/";
	CurrentPath = &Path;
	OnPathChanged();
}
#endif