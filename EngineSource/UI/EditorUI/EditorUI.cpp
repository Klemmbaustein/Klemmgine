#ifdef EDITOR
// MSVC does not like the strerror() at line 703, without this it will throw a compiler error
#define _CRT_SECURE_NO_WARNINGS
#include "EditorUI.h"
#include "Utility/stb_image.h"
#include "Engine/FileUtility.h"
#include <filesystem>
#include <algorithm>
#include "Math/Math.h"
#include <fstream>
#include "Math/Collision/Collision.h"
#include "Engine/Importers/ModelConverter.h"
#include "Engine/Importers/Importer.h"
#include "Objects/MeshObject.h"
#include "Engine/Scene.h"
#include <Engine/Input.h>
#include <Engine/Console.h>
#include <Engine/Log.h>
#include <Engine/OS.h>
#include <Engine/EngineProperties.h>
#include <thread>
#include <Engine/Importers/Build/Build.h>
#include <World/Assets.h>
#include <World/Graphics.h>
#include <World/Stats.h>
#include <UI/UIScrollBox.h>	
#include <Rendering/Texture/Texture.h>
#include <UI/EditorUI/UIVectorField.h>
#include <UI/EditorUI/MaterialTab.h>
#include <UI/EditorUI/CubemapTab.h>
#include <Objects/ParticleObject.h>
#include <Objects/InstancedMeshObject.h>
#include <Objects/SoundObject.h>
#include <Rendering/Mesh/InstancedModel.h>
#include <UI/EditorUI/PreferenceTab.h>
#include <GL/glew.h>

namespace Editor
{
	EditorUI* CurrentUI = nullptr;
}

namespace EngineUIVariables
{
	std::vector<Tab> Tabs;
}

// Collision model for the arrows

Collision::Box ArrowBoxX
(
	 0.0f, 1.0f,
	-0.1f, 0.1f,
	-0.1f, 0.1f

);

Collision::Box ArrowBoxY
(
	-0.1f, 0.1f,
	 0.0f, 1.0f,
	-0.1f, 0.1f

);

Collision::Box ArrowBoxZ
(
	-0.1f, 0.1f,
	-0.1f, 0.1f,
	-1.0f, 0.0f
	
);
bool Copied = false;
bool UserDraggingButton = false;

namespace ContentBrowser
{

	std::string CurrentFilePath = "Content";

	std::vector<Asset> ContentAssets;
	void UpdateContentAssets() //Search for assets in path
	{
		ContentAssets.clear();
		std::filesystem::path Path(CurrentFilePath);
		if (std::filesystem::exists(Path))
		{
			for (const auto& entry : std::filesystem::directory_iterator(Path))
			{
				if (std::filesystem::is_directory(entry.path()))
				{
					Asset NewAsset = Asset();
					NewAsset.FilePath = std::string(entry.path().string());
					NewAsset.IsDirectory = true;
					ContentAssets.push_back(NewAsset);
				}
			}
			for (const auto& entry : std::filesystem::directory_iterator(Path))
			{
				if (!std::filesystem::is_directory(entry.path()))
				{
					Asset NewAsset = Asset();
					NewAsset.FilePath = std::string(entry.path().string());
					NewAsset.IsDirectory = false;
					ContentAssets.push_back(NewAsset);
				}
			}
		}
		else
		{
			Log::Print(std::string("Error: Content File Path does not exist! - Resetting File Path."), Vector3(1, 0.1, 0.1));
			CurrentFilePath = "Content";
		}
	}
}

bool ChangedScene = false;
bool Dragging = false;
Vector3 PreviosLocation;
Vector3 Axis;
int SelectedTab = 0;

bool AddItemDrowdownHovered = false;

void CreateNewFile(std::string Name) //Creates a new file. Will append a number if the file already exists
{
	if (!std::filesystem::is_regular_file(Name) && !std::filesystem::is_character_file(Name) && !std::filesystem::is_other(Name))
	{
		std::fstream Out(Name, std::ios::out);
		Out.close();
		return;
	}
	int AppendedNumber = 0;
	while(true)
	{
		std::string NewName = GetFilePathWithoutExtension(Name);
		NewName.append(std::to_string(AppendedNumber));
		std::string Ext = Name.substr(Name.find_last_of(".") + 1);
		NewName.append(".").append(Ext);
		if (!std::filesystem::is_regular_file(NewName))
		{
			std::fstream Out(NewName, std::ios::out);
			Out.close();
			break;
		}
		AppendedNumber++;
	}
}

GLuint GetTextureFromFilePath(std::string Path, std::vector<GLuint> Textures) //Returns the TextureID from the given Path
{
	if (std::filesystem::is_directory(Path))
	{
		return Textures[5];
	}
	std::string Ext = Path.substr(Path.find_last_of(".") + 1);
	if (Ext == "jsm")				//3d Model
	{
		return Textures[11];
	}
	else if (Ext == "cpp")			//C++ File (Don't know what this is doing here)
	{
		return Textures[4];
	}
	else if (Ext == "txt")			//Text File (Also don't know why this is here)
	{
		return Textures[4];
	}
	else if (Ext == "png")			//Image
	{
		return Textures[18];
	}
	else if (Ext == "jscn")			//Scene
	{
		return Textures[7];
	}
	else if (Ext == "subscn")		//Subscene (Basically a Scene but it can be loaded as a subscene)
	{
		return Textures[4];
	}
	else if (Ext == "jsmat")		//Material
	{
		return Textures[9];
	}
	else if (Ext == "jsmtmp")		//Material Template
	{
		return Textures[10];
	}
	else if (Ext == "jss")			//Script
	{
		return Textures[4];
	}
	else if (Ext == "wav")			//Wave file, sound
	{
		return Textures[6];
	}
	else if (Ext == "cbm")
	{
		return Textures[4];
	}
	return Textures.at(4);		//Not recognised
}

std::vector<WorldObject*> SelectedObjects;
size_t LastConsoleLength;

UIButton* EditorUI::MakeIconButton(UIBox* Parent, unsigned int Texture, std::string Name, unsigned int ButtonIndex)
{

	/*
	The "Button surrounding box" (I couldn't think of a better name for it) is a box that surrounds the button,
	so text can be put below it (like this). An example would be the "save scene" button in the editor.
	_________________
	|  ___________  |
	|  |  Main   |  |
	|  | button  |  |
	|  -----------  |
	|      BSR      |
	|     [Text]    |
	|      BSR      |
	-----------------
	*/
	auto ButtonSurroundingBox = new UIBackground(false, 0, UIColors[0] * 1.2);
	ButtonSurroundingBox->Align = UIBox::E_DEFAULT;
	ButtonSurroundingBox->SetPadding(0, 0.01, 0, 0.05);
	ButtonSurroundingBox->SetBorder(UIBox::E_ROUNDED, 1);
	//Save scene button / [SAV]
	auto ButtonAlignBox = new UIBox(true, 0);
	ButtonAlignBox->Align = UIBox::E_CENTERED;
	ButtonAlignBox->SetPadding(0.04, 0, 0, 0);
	ButtonAlignBox->SetMinSize(Vector2(0.25, 0.16) / 2.f);
	auto NewButton = new UIButton(true, 0, Vector3(1, 1, 1), this, ButtonIndex);
	NewButton->SetMinSize(Vector2(0.09, 0.16) / 2.f);
	NewButton->SetUseTexture(true, Texture);
	NewButton->SetPadding(0);
	ButtonSurroundingBox->AddChild(new UIText(0.5, 1, Name, &EngineUIText));
	ButtonAlignBox->AddChild(NewButton);
	ButtonSurroundingBox->AddChild(ButtonAlignBox);
	Parent->AddChild(ButtonSurroundingBox);
	return NewButton;
}

EditorUI::EditorUI() : UICanvas()
{
	ArrowFramebuffer = new FramebufferObject();
	ArrowModel = new Model("EditorContent/Models/Arrows.jsm");
	ArrowModel->ModelTransform.Scale = Vector3(1, 1, -1);
	ArrowModel->ModelTransform.Rotation.Y = -M_PI_2;
	ArrowFramebuffer->UseWith(ArrowModel);
	ArrowFramebuffer->FramebufferCamera = Graphics::MainCamera;
	OutlineFramebuffer = new FramebufferObject();
	OutlineFramebuffer->FramebufferCamera = Graphics::MainCamera;

	Editor::CurrentUI = this;
	CPPClasses = GetEditorUIClasses();
	GenUITextures();
	Input::CursorVisible = true;
	//Create the viewport Tab
	EngineUIVariables::Tabs.push_back(EngineUIVariables::Tab(0, "Viewport", "", false));

	TabItems =
	{
		nullptr,
		new MeshTab(UIColors, &EngineUIText),
		new MaterialTemplateTab(UIColors, &EngineUIText, Textures[4]),
		new MaterialTab(UIColors, &EngineUIText, Textures[12]),
		new ParticleEditorTab(UIColors, &EngineUIText, Textures[4], Textures[12]),
		new PreferenceTab(UIColors, &EngineUIText),
		new CubemapTab(UIColors, &EngineUIText)
	};

	//Scan for assets
	ContentBrowser::UpdateContentAssets();
	//Initialize all the UI Elements

	auto TopBar = new UIBackground(true, Vector2(-1, 0.925), UIColors[1], Vector2(2.f, 0.075f));
	TopBar->Align = UIBox::E_DEFAULT;
	TopBarText[0] = new UIText(0.7, Vector3(0.8), "FPS: 60", &EngineUIText);
	TopBarText[0]->SetTryFill(true);
	TopBarText[0]->SetPadding(0.02, 0.015, 0.02, 0.02);
	TopBar->AddChild(TopBarText[0]);
	auto SmallTopTextBox = new UIBox(false, 0);
	TopBarText[1] = new UIText(0.4, Vector3(0.7), ProjectName + std::string(" Editor"), &EngineUIText);
	TopBarText[2] = new UIText(0.4, Vector3(0.7), "Engine Ver: " + std::string(VERSION_STRING), &EngineUIText);
	TopBarText[1]->SetPadding(0);
	TopBarText[2]->SetPadding(0);
	SmallTopTextBox->SetPadding(0, 0.02, 0, 0);
	SmallTopTextBox->AddChild(TopBarText[1]);
	SmallTopTextBox->AddChild(TopBarText[2]);
	SmallTopTextBox->SetTryFill(true);
	TopBar->AddChild(SmallTopTextBox);

	//This is for the top background
	/*
	* ANI = Add new item button
	* IMP = Import button
	* SAV = Save button
	* PAK = Package project
	* WIR = Wireframe button
	* PREF = Preference button
	* -------------------------------------------
	* |   [IMP]     [SAV] [PAK] [WIR] [PREF]    |
	* -------------------------------------------
	*/

	auto TopBackground = new UIBackground(true, Vector2(-1, 0.725), UIColors[0], Vector2(2.f, 0.2f));
	//Add new item button / [ANI]

	//Import button / [IMP]
	auto NewButton = new UIButton(true, 0, Vector3(0, 1, 0), this, 0);
	NewButton->AddChild(new UIText(0.6, Vector3(0), "Import", &EngineUIText));
	NewButton->SetPadding(0, 0.04, 0.1, 0.1);
	NewButton->SetBorder(UIBox::E_ROUNDED, 1);
	TopBackground->AddChild(NewButton);

	//SAV, PAK, WIR, PREF buttons.
	MakeIconButton(TopBackground, Textures[2], "Save scene", 2);
	MakeIconButton(TopBackground, Textures[1], "Wireframe", 7);
	//MakeIconButton(TopBackground, Textures[15], "Preferences", 8);
	MakeIconButton(TopBackground, Textures[3], "Package", 3);

	TabBox = new UIBackground(true, Vector2(-0.7, 0.675), UIColors[1], Vector2(1.4f, 0.05f));


	//This is the left horizontal editor bar, where the item browser is located.
	/*
	* BAR = Top bar of the item browser
	* ITM = The item list
	* MDE = The item browser mode selection
	* _________
	* | [BAR] |
	* | [MDE] |
	* | ----- |
	* | |ITM| |
	* | |   | |
	* | |   | |
	* | ----- |
	* ---------
	*/

	auto LeftBackground = new UIBackground(false, Vector2(-1, -1), UIColors[0], Vector2(0.3, 1.725));
	LeftBackground->Align = UIBox::E_REVERSE;

	// BAR / Top bar of the item browser
	LeftBoxes[0] = new UIBox(true, 0);
	LeftBoxes[0]->SetPadding(0);

	// MDE / item browser mode
	LeftBoxes[1] = new UIBackground(true, 0, UIColors[0] * 2);
	LeftBoxes[1]->SetPadding(0);

	// ITM container
	LeftBoxes[2] = new UIScrollBox(false, 0, 25);
	LeftBoxes[2]->SetMinSize(Vector2(0, 1.6));
	LeftBoxes[2]->SetMaxSize(Vector2(0.3, 1.6));
	LeftBoxes[2]->SetPadding(0);
	LeftBoxes[2]->Align = UIBox::E_REVERSE;
	LeftBackground->AddChild(LeftBoxes[0]);
	LeftBackground->AddChild(LeftBoxes[1]);
	LeftBackground->AddChild(LeftBoxes[2]);


	//Log
	auto LowerBackground = (new UIBackground(true, Vector2(-0.7, -1), UIColors[0], Vector2(1.41, 0.4)))->SetMaxSize(Vector2(1.41, 0.41));
	auto LogBackground = new UIBackground(false, 0, UIColors[1], Vector2(1, 0));
	LowerBackground->AddChild(LogBackground);
	LogScrollObject = new UIScrollBox(false, 0, 100);
	LogScrollObject->SetPadding(0);
	LogScrollObject->SetMinSize(Vector2(1, 0.32));
	LogConsolePromt = new UITextField(true, 0, UIColors[1] * 0.6, this, 6, &EngineUIText);
	LogConsolePromt->SetPadding(0);
	LogConsolePromt->SetMinSize(Vector2(0.9, 0.05));
	LogConsolePromt->HintText = "Enter command here";
	LogConsolePromt->SetBorder(UIBox::E_ROUNDED, 0.5);
	LogConsolePromt->SetTryFill(true);
	LogBackground->SetBorder(UIBox::E_ROUNDED, 0.5);
	LogBackground->AddChild(LogConsolePromt);
	LogBackground->AddChild(LogScrollObject);


	auto RightBackground = new UIBackground(false, Vector2(0.7, -1), UIColors[0], Vector2(0.3, 1.725));
	auto NewBackground = new UIBackground(false, 0, UIColors[1] * 1.2, Vector2(0.3, 1));
	NewBackground->SetPadding(0);
	NewBackground->SetMinSize(Vector2(0.3, 1));
	NewBackground->Align = UIBox::E_REVERSE;
	NewBackground->SetMaxSize(Vector2(0.3, 1));
	NewBackground->AddChild((new UIText(0.6, Vector3(1), " Objects", &EngineUIText))->SetPadding(0.005));
	RightBackground->AddChild(NewBackground);
	RightBackground->Align = UIBox::E_REVERSE;

	RightBoxes[0] = new UIScrollBox(false, Vector2(0.725, -0.275), 150);
	RightBoxes[0]->Align = UIBox::E_REVERSE;
	RightBoxes[0]->SetMinSize(Vector2(0.3, 0.95));
	RightBoxes[0]->SetMaxSize(Vector2(0.3, 0.95));
	RightBoxes[0]->SetPadding(0);

	RightBoxes[1] = new UIScrollBox(false, 0, 100);
	auto SeperatorBar = new UIBackground(true, 0, UIColors[0] + 0.15f, Vector2(0.3, 0.01));
	SeperatorBar->SetPadding(0);
	RightBackground->AddChild(SeperatorBar);
	RightBackground->AddChild(RightBoxes[1]);
	RightBoxes[1]->SetMinSize(Vector2(0.3, 0.7));
	RightBoxes[1]->SetMaxSize(Vector2(0.3, 0.7));
	RightBoxes[1]->Align = UIBox::E_REVERSE;
	RightBoxes[1]->SetPadding(0);

	//Create cursors
	CrossCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	GrabCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	DefaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	LoadCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	UpdateItemBrowser();
	UpdateObjectList();
	UpdateTabs();
	UpdateContextMenu();
}

void(*QuitF)();
void EditorUI::OnLeave(void(*ReturnF)())
{
	QuitF = ReturnF;
	if (ChangedScene)
	{
		ShowPopUpWindow("Save Changes to scene?",
			{
				PopUpButton("Yes", true, []() { Scene::SaveSceneAs(Scene::CurrentScene, Editor::IsInSubscene); QuitF(); }),
				PopUpButton("No", true, QuitF),
				PopUpButton("Cancel", false, nullptr)
			});
	}
	else QuitF();
}

void EditorUI::ShowPopUpWindow(std::string Message, std::vector<PopUpButton> Buttons)
{
	PopUpMesage = Message;
	if (PopUpMenu) delete PopUpMenu;
	PopUpMenu = new UIBackground(false, Vector2(-0.2, -0.1), UIColors[0] * 0.8, Vector2(0.4, 0.2));
	PopUpMenu->SetBorder(UIBox::E_ROUNDED, 1);
	PopUpMenu->Align = UIBox::E_REVERSE;
	PopUpMenu->AddChild(new UIText(0.6, UIColors[2], Message, &EngineUIText));
	PopUpButtons = Buttons;
	auto OptionsBox = new UIBox(true, 0);
	PopUpMenu->AddChild(OptionsBox);
	for (size_t i = 0; i < Buttons.size(); i++)
	{
		auto NewButton = new UIButton(true, 0, UIColors[2], this, i + 22);
		NewButton->SetPadding(0.005, 0.01, 0.005, 0.005);
		NewButton->AddChild((new UIText(0.5, UIColors[0], Buttons[i].Text, &EngineUIText))->SetPadding(0.01));
		OptionsBox->AddChild(NewButton);
		NewButton->SetBorder(UIBox::E_ROUNDED, 0.5);
	}
}

void EditorUI::OnButtonClicked(int Index)
{
	if (Index >= 200 && Index < 300 && !ContentBrowserMode)
	{
		int ItemBrowserIndex = Index - 200;
		std::string Ext = GetExtension(ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
		if (!UserDraggingButton)
		{
			if (Ext == "jsm")
			{
				OpenTab(1, ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
			}
			if (Ext == "jsmtmp")
			{
				OpenTab(2, ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
			}
			if (Ext == "jsmat")
			{
				OpenTab(3, ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
			}
			if (Ext == "jscn")
			{
				SelectedObjects.clear();
				Scene::LoadNewScene(ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
				Scene::Tick();
				ChangedScene = false;
				UpdateObjectList();
				UpdateContextMenu();
			}
			if (Ext == "png")
			{
				OS::OpenFile(ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
			}
			if (Ext == "wav")
			{
				Console::ExecuteConsoleCommand("playsound " + GetFileNameWithoutExtensionFromPath(ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath));
			}
			if (Ext == "jspart")
			{
				OpenTab(4, ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
			}
			if (Ext == "cbm")
			{
				OpenTab(6, ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath);
			}
			if (ContentBrowser::ContentAssets[ItemBrowserIndex].IsDirectory)
			{
				ContentBrowser::CurrentFilePath = ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath;
				ContentBrowser::UpdateContentAssets();
				UpdateItemBrowser();
			}
		}
		else if(ContentBrowser::ContentAssets[ItemBrowserIndex].IsDirectory)
		{
			auto Name = GetFileNameFromPath(ContentBrowser::ContentAssets[DraggedButton].FilePath);
			if (std::filesystem::exists(ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath + "/" + Name))
			{
				std::filesystem::remove(ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath + "/" + Name);
			}
			std::filesystem::rename(ContentBrowser::ContentAssets[DraggedButton].FilePath,
				ContentBrowser::ContentAssets[ItemBrowserIndex].FilePath + "/" + Name);
			UpdateItemBrowser();
		}
	}
	else if (Index >= 200 && Index < 300)
	{
		auto ObjectsInFolder = GetContentsOfCurrentCPPFolder();
		if (ObjectsInFolder[Index - 200].IsFolder)
		{
			CPPPath.push_back(Index - 200);
			UpdateItemBrowser();
		}
	}
	if (Index >= 300 && Index < 1000)
	{
		for (auto o : Objects::AllObjects)
		{
			o->IsSelected = false;
		}
		int ObjectIndex = Index - 300;
		Objects::AllObjects[ObjectIndex]->IsSelected = true;
		UpdateContextMenu();
	}
	if (Index >= 1000 && Index < 1100)
	{
		TextInput::PollForText = true;
		TextInput::Text = GetFileNameWithoutExtensionFromPath(ContentBrowser::ContentAssets[Index - 1000].FilePath);
		TextInput::TextIndex = TextInput::Text.size();
		RenamingContentAsset = true;
		RenamedIndex = Index - 1000;
	}
	if (Index >= 1100 && Index < 1200)
	{
		int FileIndex = Index - 1100;
		std::filesystem::remove_all(ContentBrowser::ContentAssets[FileIndex].FilePath);
		UpdateItemBrowser();
	}
	if (Index >= 1200 && Index < 1300)
	{
		int FileIndex = Index - 1200;
		std::string File = ContentBrowser::ContentAssets[FileIndex].FilePath;
		std::string NewFile = GetFilePathWithoutExtension(File) + "_Copy";
		if (!GetExtension(File).empty()) NewFile.append("." + GetExtension(File));
		std::filesystem::copy(File, NewFile);
		UpdateItemBrowser();
	}
	if (Index >= 1300 && Index < 1400)
	{
		std::string Elem = "OBJ_CAT_" + ObjectCategories[Index - 1300];
		if (CollapsedItems.contains(Elem))
		{
			CollapsedItems.erase(Elem);
		}
		else
		{
			CollapsedItems.insert(Elem);
		}
		UpdateObjectList();
	}
	if (Index >= -60 && Index < -10)
	{
		std::string Prefix = SelectedObjects.size() ? "OBJ_CAT_" : "SCN_";
		std::string cat = SceneCategories[Index + 60];
		if (CollapsedItems.contains(Prefix + cat))
		{
			CollapsedItems.erase(Prefix + cat);
		}
		else
		{
			CollapsedItems.insert(Prefix + cat);
		}
		UpdateContextMenu();
	}
	if (Index >= -200 && Index < -100)
	{
		int TabIndex = Index + 200;
		if (TabIndex % 2)
		{
			if (SelectedTab >= TabIndex / 2)
			{
				if (TabItems[EngineUIVariables::Tabs[SelectedTab].Type])
				{
					TabItems[EngineUIVariables::Tabs[SelectedTab].Type]->Save();
				}
				SelectedTab--;
				if (TabItems[EngineUIVariables::Tabs[SelectedTab].Type])
				{
					TabItems[EngineUIVariables::Tabs[SelectedTab].Type]->Load(EngineUIVariables::Tabs[SelectedTab].Path);
				}
			}
			EngineUIVariables::Tabs.erase(EngineUIVariables::Tabs.begin() + TabIndex / 2);
		}
		else
		{
			if (TabItems[EngineUIVariables::Tabs[SelectedTab].Type])
			{
				TabItems[EngineUIVariables::Tabs[SelectedTab].Type]->Save();
			}
			SelectedTab = TabIndex / 2;
			if (TabItems[EngineUIVariables::Tabs[SelectedTab].Type])
			{
				TabItems[EngineUIVariables::Tabs[SelectedTab].Type]->Load(EngineUIVariables::Tabs[SelectedTab].Path);
			}
		}
		UpdateTabs();
	}
	if (Index > 21 && Index < 25 && PopUpMenu)
	{
		delete PopUpMenu;
		PopUpMenu = nullptr;
		if (PopUpButtons[Index - 22].CallFunc)
		{
			void(*ButtonPtr)() = PopUpButtons.at(Index - 22).FPointer;
			(*ButtonPtr)();
		}
	}
	if (Index == 0)
	{
		std::string SelectedFile = OS::ShowOpenFileDialog();
		std::string Extension = GetExtension(SelectedFile);
		if (Extension == "wav" || Extension == "png")
		{
			Importer::Import(SelectedFile, ContentBrowser::CurrentFilePath);
		}
		if (Extension == "fbx" || Extension == "obj")
		{
			ModelImporter::Import(SelectedFile, ContentBrowser::CurrentFilePath);
		}
		UpdateItemBrowser();
	}
	if (Index == 2)
	{
		Scene::SaveSceneAs(Scene::CurrentScene, Editor::IsInSubscene);
		ChangedScene = false;
		Log::Print("Saved scene " + Scene::CurrentScene);
	}
	if (Index == 3)
	{
		BuildThread = new std::thread(Build::TryBuildProject, "Build/");
	}
	if (Index == 4)
	{
		std::string NewPath = ItemBrowserPath->GetText();
		if (std::filesystem::is_directory(NewPath))
		{
			ContentBrowser::CurrentFilePath = NewPath;
			UpdateItemBrowser();
		}
		else Log::Print("The given content browser path does not exist.");
	}
	if (Index == 5)
	{
		if (!ContentBrowserMode)
		{
			if (!UserDraggingButton && ContentBrowser::CurrentFilePath != "./Content")
			{
				auto lastindex = ContentBrowser::CurrentFilePath.find_last_of("/\\");
				if (lastindex != std::string::npos)
				{
					std::string rawname = ContentBrowser::CurrentFilePath.substr(0, lastindex);
					ContentBrowser::CurrentFilePath = rawname;
					UpdateItemBrowser();
				}
			}
			else if (UserDraggingButton)
			{
				size_t lastindex = ContentBrowser::CurrentFilePath.find_last_of("/\\");
				std::string rawname = ContentBrowser::CurrentFilePath.substr(0, lastindex);
				if (std::rename(ContentBrowser::ContentAssets.at(DraggedButton).FilePath.c_str(),
					(rawname).append("/").append(GetFileNameFromPath(ContentBrowser::ContentAssets.at(DraggedButton).FilePath)).c_str()) < 0)
					Log::Print(std::string("Error moving File: ").append(strerror(errno)), Vector3(1, 0.3f, 0.3f));
				UpdateItemBrowser();
			}
		}
		else
		{
			if (CPPPath.size())
			{
				CPPPath.pop_back();
				UpdateItemBrowser();
			}
		}
	}
	if (Index == 6)
	{
		if (Input::IsKeyDown(SDLK_RETURN) && !LogConsolePromt->GetText().empty())
		{
			Log::Print("> " + LogConsolePromt->GetText(), Vector3(0.5, 0.6, 1));
			Console::ExecuteConsoleCommand(LogConsolePromt->GetText(), true);
			LogConsolePromt->SetText("");
			TextInput::PollForText = false;
		}
	}
	if (Index == 7)
	{
		Graphics::IsWireframe = !Graphics::IsWireframe;
	}
	if (Index == 8)
	{
		OpenTab(5, "Preferences");
	}
	if (Index == 20 || Index == 21)
	{
		ContentBrowserMode = Index - 20;
		UpdateItemBrowser();
	}
	if (Index == 25)
	{
		CreateNewFile(ContentBrowser::CurrentFilePath + "/Material.jsmat");
		UpdateItemBrowser();
	}
	if (Index == 26)
	{
		CreateNewFile(ContentBrowser::CurrentFilePath + "/MaterialTemplate.jsmtmp");
		UpdateItemBrowser();
	}
	if (Index == 27)
	{
		std::filesystem::create_directory(ContentBrowser::CurrentFilePath + "/Folder");
		UpdateItemBrowser();
	}
	if (Index == 28)
	{
		CreateNewFile(ContentBrowser::CurrentFilePath + "/Scene.jscn");
		UpdateItemBrowser();
	}
	if (Index == 29)
	{
		CreateNewFile(ContentBrowser::CurrentFilePath + "/Particle.jspart");
		UpdateItemBrowser();
	}
	if (Index == 30)
	{
		CreateNewFile(ContentBrowser::CurrentFilePath + "/Cubemap.cbm");
		UpdateItemBrowser();
	}
	if (Index == 44) // Context menu property changed
	{
		for (size_t i = 0; i < SceneButtons.size(); i++)
		{
			switch (SceneSettings[i].Type)
			{
			case Type::E_VECTOR3_COLOR:
			case Type::E_VECTOR3:
				if(SceneSettings[i].Normalized) *(Vector3*)(SceneSettings[i].Variable) = ((UIVectorField*)SceneButtons[i])->GetValue().Normalize();
				else
				*(Vector3*)(SceneSettings[i].Variable) = ((UIVectorField*)SceneButtons[i])->GetValue();
				break;
			case Type::E_FLOAT:
				*(float*)(SceneSettings[i].Variable) = std::stof(((UITextField*)SceneButtons[i])->GetText());
				break;
			case Type::E_INT:
				*(int*)(SceneSettings[i].Variable) = std::stof(((UITextField*)SceneButtons[i])->GetText());
				break;
			case Type::E_STRING:
				*(std::string*)(SceneSettings[i].Variable) = ((UITextField*)SceneButtons[i])->GetText();
				if (SelectedObjects.size() && SceneSettings[i].Variable == &SelectedObjects[0]->Name)
				{
					UpdateObjectList();
				}
				break;
			case Type::E_BOOL:
				if (((UIButton*)SceneButtons[i])->GetIsHovered())
				{
					*(bool*)SceneSettings[i].Variable = !(*(bool*)SceneSettings[i].Variable);
				}
				break;
			default:
				break;
			}
		}
		if (SelectedObjects.size() && SelectedObjects[0]->Properties.size()) SelectedObjects[0]->OnPropertySet();
		ChangedScene = true;
		UpdateContextMenu();
	}
}

void EditorUI::OnButtonDragged(int Index)
{
	if (!UserDraggingButton)
	{
		Log::Print("HI");
		auto ObjectsInFolder = GetContentsOfCurrentCPPFolder();
		Index -= 200;
		if (ContentBrowserMode && ObjectsInFolder.size() < Index) return;
		if (ContentBrowserMode && ObjectsInFolder[Index].IsFolder) return;

		DraggedButton = Index;
		UserDraggingButton = true;
		DragUI = new UIBackground(false, 0, Vector3(UIColors[0] * 1.5));
		DragUI->SetMinSize(Vector2(0.1, 0.25) / 1.2f);
		DragUI->SetPadding(0.16 / 20.0f, 0.16 / 20.0f, 0.09 / 20.0f, 0.09 / 20.0f);
		DragUI->Align = UIBox::E_REVERSE;
		auto ItemBrowserImage = new UIBackground(true, 0, 1);
		std::string Extension;
		unsigned int Texture = 0;
		std::string Name;
		if (!ContentBrowserMode)
		{
			Extension = GetExtension(ContentBrowser::ContentAssets[Index].FilePath);
			if (std::filesystem::is_directory(ContentBrowser::ContentAssets[Index].FilePath))
			{
				Extension = "dir";
			}
			Name = GetFileNameWithoutExtensionFromPath(ContentBrowser::ContentAssets[Index].FilePath);
			Texture = GetTextureFromFilePath(ContentBrowser::ContentAssets[Index].FilePath, Textures);
		}
		else
		{

			Name = ObjectsInFolder[Index].Name;
			Extension = "cpp";
			Texture = Textures[0];
		}
		
		Vector3 IconColor = Vector3(1, 0.1, 0);
		auto FoundColor = IconColors.find(Extension);
		if (FoundColor != IconColors.end())
		{
			IconColor = FoundColor->second;
		}
		auto ItemBrowserImageBackground = new UIBackground(true, 0, IconColor);
		ItemBrowserImageBackground->SetPadding(0.16 / 20.0f, 0.16 / 20.0f, 0.09 / 20.0f, 0.09 / 20.0f);
		ItemBrowserImageBackground->AddChild(ItemBrowserImage);
		ItemBrowserImage->SetPadding(0);
		ItemBrowserImage->SetMinSize(Vector2(0.09 / 1.2f, 0.16 / 1.2f));
		ItemBrowserImage->SetUseTexture(true, Texture);
		auto Text = new UIText(0.45, 1, Name, &EngineUIText);
		DragUI->AddChild(ItemBrowserImageBackground);
		DragUI->AddChild(Text);
		Text->SetPadding(0.001);
		Text->Wrap = true;
		Text->WrapDistance = 0.1;
	}
}

void EditorUI::Tick()
{
	auto& CurrentTab = EngineUIVariables::Tabs[SelectedTab];
	if (!TabItems[CurrentTab.Type] || dynamic_cast<MeshTab*>(TabItems[CurrentTab.Type]) || dynamic_cast<ParticleEditorTab*>(TabItems[CurrentTab.Type]) || 
		dynamic_cast<CubemapTab*>(TabItems[CurrentTab.Type]))
	{
		if (Input::IsRMBDown && !RMouseLock && Maths::IsPointIn2DBox(Vector2(0.7f, 0.8f), Vector2(-0.7f, -0.6f), Input::MouseLocation))
			RMouseLock = true;
		if (!Input::IsRMBDown)
			RMouseLock = false;
		OutlineFramebuffer->Renderables.clear();
		if (RMouseLock)
			SDL_SetRelativeMouseMode(SDL_TRUE);
		else
			if (Input::CursorVisible)
				SDL_SetRelativeMouseMode(SDL_FALSE);
			else
				SDL_SetRelativeMouseMode(SDL_TRUE);
		if (RMouseLock)
		{
			Graphics::MainCamera->OnMouseMoved(Input::MouseMovement.X * 6, -Input::MouseMovement.Y * 6);
			float Speed = Input::IsKeyDown(SDLK_LSHIFT) ? 175 : (Input::IsKeyDown(SDLK_LCTRL) ? 15 : 50);
			if (Input::IsKeyDown(SDLK_w))
			{
				Graphics::MainCamera->MoveForward(Performance::DeltaTime * Speed);
			}
			if (Input::IsKeyDown(SDLK_s))
			{
				Graphics::MainCamera->MoveForward(Performance::DeltaTime * -Speed);
			}
			if (Input::IsKeyDown(SDLK_a))
			{
				Graphics::MainCamera->MoveRight(Performance::DeltaTime * -Speed);
			}
			if (Input::IsKeyDown(SDLK_d))
			{
				Graphics::MainCamera->MoveRight(Performance::DeltaTime * Speed);
			}
			if (Input::IsKeyDown(SDLK_q))
			{
				Graphics::MainCamera->MoveUp(Performance::DeltaTime * -Speed);
			}
			if (Input::IsKeyDown(SDLK_e))
			{
				Graphics::MainCamera->MoveUp(Performance::DeltaTime * Speed);
			}
		}
	}


	SelectedObjects.clear();
	for (auto* i : Objects::AllObjects)
	{
		if (i->IsSelected)
		{
			SelectedObjects.push_back(i);
			if (!(!TabItems[CurrentTab.Type] || dynamic_cast<MeshTab*>(TabItems[CurrentTab.Type]) || dynamic_cast<ParticleEditorTab*>(TabItems[CurrentTab.Type]))) continue;
			for (auto* c : i->Components)
			{
				if (dynamic_cast<MeshComponent*>(c))
					OutlineFramebuffer->Renderables.push_back(dynamic_cast<MeshComponent*>(c)->GetModel());
				if (dynamic_cast<InstancedMeshComponent*>(c))
					OutlineFramebuffer->Renderables.push_back(dynamic_cast<InstancedMeshComponent*>(c)->GetInstancedModel());
			}
		}
	}
	ArrowModel->Visible = SelectedObjects.size();
	if (SelectedObjects.size())
	{
		ArrowModel->ModelTransform.Location = SelectedObjects[0]->GetTransform().Location;
		ArrowModel->ModelTransform.Scale = Vector3(1, 1, -1) * Vector3::Distance(Graphics::MainCamera->Position, ArrowModel->ModelTransform.Location) * 0.025f;
		ArrowModel->UpdateTransform();
	}
	if (SelectedObjects.size() && SelectedObjects[0] != PreviousSelectedObject)
	{
		PreviousSelectedObject = SelectedObjects[0];
		dynamic_cast<UIScrollBox*>(RightBoxes[1])->GetScrollObject()->Percentage = 0;
		UpdateContextMenu();
		UpdateObjectList();
	}
	else if (!SelectedObjects.size() && PreviousSelectedObject)
	{
		PreviousSelectedObject = nullptr;
		UpdateContextMenu();
		UpdateObjectList();
	}

	if (RenamingContentAsset)
	{
		if (!TextInput::PollForText)
		{
			std::string NewName = ContentBrowser::ContentAssets[RenamedIndex].FilePath
				.substr(0, ContentBrowser::ContentAssets[RenamedIndex].FilePath.find_last_of("/\\"));
			NewName.append("/" + TextInput::Text);
			if (!ContentBrowser::ContentAssets[RenamedIndex].IsDirectory)
			{
				NewName.append("." + GetExtension(ContentBrowser::ContentAssets[RenamedIndex].FilePath));
			}
			std::filesystem::rename(ContentBrowser::ContentAssets[RenamedIndex].FilePath, NewName);
			RenamingContentAsset = false;
			ContentBrowser::UpdateContentAssets();
			UpdateItemBrowser();
		}
		else
		{
			std::string DisplayedString = TextInput::Text + " ";
			ItemBrowserButtons[RenamedIndex]->SetColor(Vector3(0.5, 0.2, 0));
			DisplayedString[TextInput::TextIndex] = '|';
			ItemBrowserTexts[RenamedIndex]->SetText(DisplayedString);
		}
	}

	for (int i = 0; i < TabItems.size(); i++)
	{
		if (TabItems[i])
		{
			TabItems[i]->SetVisible(EngineUIVariables::Tabs[SelectedTab].Type == i);
		}
	}
	Vector3 DistanceScaleMultiplier;
	Vector3 Rotation = Graphics::MainCamera->ForwardVectorFromScreenPosition(Input::MouseLocation.X * 1.4f, ((Input::MouseLocation.Y * 1.5f) - 0.1f));
	if (SelectedObjects.size() > 0)
		DistanceScaleMultiplier = Vector3((SelectedObjects.at(0)->GetTransform().Location - Vector3::Vec3ToVector(Graphics::MainCamera->Position)).Length() * 0.15f);
	if (Input::IsLMBDown && !LMBDown)
	{
		LMBDown = true;
		if (Maths::IsPointIn2DBox(Vector2(0.7f, 0.675f), Vector2(-0.7f, -0.6f), Input::MouseLocation) && !SelectedTab && !UI::HoveredButton)
		{
			bool Hit = false;
			if (SelectedObjects.size() > 0)
			{
				float t = INFINITY;
				Collision::HitResponse
				CollisionTest = Collision::LineCheckForAABB((ArrowBoxZ * DistanceScaleMultiplier) + SelectedObjects.at(0)->GetTransform().Location,
					Vector3::Vec3ToVector(Graphics::MainCamera->Position), (Rotation * 500.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position));
				if (CollisionTest.Hit)
				{
					PreviosLocation = Rotation;
					Hit = true;
					Dragging = true;
					Axis = Vector3(0, 0, 1.f);
					t = CollisionTest.t;
				}
				CollisionTest = Collision::LineCheckForAABB((ArrowBoxY * DistanceScaleMultiplier) + SelectedObjects.at(0)->GetTransform().Location,
					Vector3::Vec3ToVector(Graphics::MainCamera->Position), (Rotation * 500.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position));
				if (CollisionTest.Hit && CollisionTest.t < t)
				{
					PreviosLocation = Rotation;
					Hit = true;
					Dragging = true;
					Axis = Vector3(0, 1.f, 0);
					t = CollisionTest.t;
				}
				CollisionTest = Collision::LineCheckForAABB((ArrowBoxX * DistanceScaleMultiplier) + SelectedObjects.at(0)->GetTransform().Location,
					Vector3::Vec3ToVector(Graphics::MainCamera->Position), (Rotation * 500.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position));
				if (CollisionTest.Hit && CollisionTest.t < t)
				{
					PreviosLocation = Rotation;
					Hit = true;
					Dragging = true;
					Axis = Vector3(1.f, 0, 0);
					t = CollisionTest.t;
				}
			}

			Collision::HitResponse CollisionTest = Collision::LineTrace(Vector3::Vec3ToVector(Graphics::MainCamera->Position),
				(Rotation * 50000.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position));
			if (CollisionTest.Hit && !Hit)
			{
				for (int i = 0; i < SelectedObjects.size(); i++)
				{
					if (SelectedObjects[i] != CollisionTest.HitObject)
						SelectedObjects[i]->IsSelected = false;
				}
				CollisionTest.HitObject->IsSelected = true;
			}
		}
	}
	if (ShouldStopDragging == 3)
	{
		ShouldStopDragging = false;
		UserDraggingButton = false;
		delete DragUI;
		DragUI = nullptr;
	}
	else if (ShouldStopDragging)
	{
		ShouldStopDragging++;
	}
	if (!Input::IsLMBDown && LMBDown)
	{
		LMBDown = false;
		if (Dragging)
		{
			Dragging = false;
			UpdateContextMenu();
		}
		if (RenamingContentAsset)
		{
			TextInput::PollForText = false;
			std::string NewName = ContentBrowser::ContentAssets[RenamedIndex].FilePath
				.substr(0, ContentBrowser::ContentAssets[RenamedIndex].FilePath.find_last_of("/\\"));
			NewName.append("/" + TextInput::Text);
			if (!ContentBrowser::ContentAssets[RenamedIndex].IsDirectory)
			{
				NewName.append("." + GetExtension(ContentBrowser::ContentAssets[RenamedIndex].FilePath));
			}
			std::filesystem::rename(ContentBrowser::ContentAssets[RenamedIndex].FilePath, NewName);
			RenamingContentAsset = false;
			ContentBrowser::UpdateContentAssets();
			UpdateItemBrowser();
		}
		if (UserDraggingButton)
		{
			ShouldStopDragging = true;
			if (Maths::IsPointIn2DBox(Vector2(0.7f, 0.675f), Vector2(-0.7f, -0.6f), Input::MouseLocation) && !SelectedTab)
			{
				WorldObject* NewObject = nullptr;
				if (ContentBrowserMode)
				{
					auto ObjectsInFolder = GetContentsOfCurrentCPPFolder();
					Collision::HitResponse CollisionTest = Collision::LineTrace(Vector3::Vec3ToVector(Graphics::MainCamera->Position),
						(Rotation * 500.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position));
					if (CollisionTest.Hit)
					{
						NewObject = Objects::SpawnObjectFromID(ObjectsInFolder[DraggedButton].Object.ID,
							Transform(Vector3::SnapToGrid(CollisionTest.ImpactPoint, 0.5f), Vector3(0), Vector3(1.f)));
					}
					else
					{
						NewObject = Objects::SpawnObjectFromID(ObjectsInFolder[DraggedButton].Object.ID,
							Transform(Vector3::SnapToGrid((Rotation * 45) + Vector3::Vec3ToVector(Graphics::MainCamera->Position), 0.5f), Vector3(0), Vector3(1.f)));
					}
					ChangedScene = true;
				}
				else
				{
					std::string Name = GetFileNameWithoutExtensionFromPath(ContentBrowser::ContentAssets.at(DraggedButton).FilePath);
					std::string Ext = GetExtension(ContentBrowser::ContentAssets[DraggedButton].FilePath);
					Collision::HitResponse CollisionTest = Collision::LineTrace(Vector3::Vec3ToVector(Graphics::MainCamera->Position),
						(Rotation * 500.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position));
					Vector3 SpawnPosition;
					if (CollisionTest.Hit)
					{
						SpawnPosition = Vector3::SnapToGrid(CollisionTest.ImpactPoint, 0.5f);
					}
					else
					{
						SpawnPosition = Vector3::SnapToGrid((Rotation * 45.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position), 0.5f);
					}
					if (Ext == "jsm")
					{
						NewObject = Objects::SpawnObject<MeshObject>(Transform(SpawnPosition, Vector3(0), Vector3(1.f)));
						((MeshObject*)NewObject)->LoadFromFile(Name);
						NewObject->SetName(Name);
						ChangedScene = true;
						
					}
					if (Ext == "jspart")
					{
						NewObject = Objects::SpawnObject<ParticleObject>(Transform(SpawnPosition, 0, 1));
						dynamic_cast<ParticleObject*>(NewObject)->LoadParticle(Name);
						ChangedScene = true;
					}
					if (Ext == "wav")
					{
						NewObject = Objects::SpawnObject<SoundObject>(Transform(SpawnPosition, 0, 1));
						dynamic_cast<SoundObject*>(NewObject)->LoadSound(Name);
						ChangedScene = true;
					}
				}
				if(NewObject)
				NewObject->IsSelected = true;
				UpdateObjectList();
			}
		}
		else
		{
			ShouldStopDragging = false;
		}
		if (DropDownMenu)
		{
			delete DropDownMenu;
			DropDownMenu = nullptr;
		}
	}
	if (Dragging)
	{
		Vector3 TransformToAdd = ((Rotation * Axis) - (PreviosLocation * Axis)) * (Vector3(5) * DistanceScaleMultiplier);

		for (int i = 0; i < Objects::AllObjects.size(); i++)
		{
			if (Objects::AllObjects.at(i)->IsSelected)
			{
				Objects::AllObjects.at(i)->SetTransform(Objects::AllObjects.at(i)->GetTransform() + Transform(TransformToAdd, Vector3(), Vector3(1)));
			}
		}
		PreviosLocation = Rotation;
		ChangedScene = true;
		Input::CursorVisible = false;
	}
	else Input::CursorVisible = true;
	if (SelectedObjects.size() > 0 && !SelectedTab)
	{
		if (Input::IsKeyDown(SDLK_LCTRL) && Input::IsKeyDown(SDLK_w) && !Input::IsRMBDown)
		{
			if (!Copied)
			{
				WorldObject* NewObject = Objects::SpawnObjectFromID(SelectedObjects[0]->GetObjectDescription().ID, SelectedObjects[0]->GetTransform());
				NewObject->Deserialize(SelectedObjects[0]->Serialize());
				NewObject->LoadProperties(SelectedObjects[0]->GetPropertiesAsString());
				NewObject->OnPropertySet();
				NewObject->SetName(SelectedObjects[0]->GetName());
				for (int i = 0; i < Objects::AllObjects.size(); i++)
				{
					Objects::AllObjects.at(i)->IsSelected = false;
				}
				NewObject->IsSelected = true;
				UpdateObjectList();
				Copied = true;
				ChangedScene = true;
			}
		}
		else Copied = false;
	}

	if (Input::IsRMBDown)
	{
		RMBDown = true;
	}
	else if (RMBDown)
	{
		RMBDown = false;
		if (DropDownMenu)
		{
			delete DropDownMenu;
			DropDownMenu = nullptr;
		}
		if (!ContentBrowserMode)
		{
			if (UI::HoveredButton && UI::HoveredButton->GetIndex() >= 200 && UI::HoveredButton->GetIndex() < 300)
			{
				MakeDropDownMenu(Input::MouseLocation, {
						EditorDropdownItem("cat_" + GetFileNameWithoutExtensionFromPath(ContentBrowser::ContentAssets[UI::HoveredButton->GetIndex() - 200].FilePath), 0, 1),
						EditorDropdownItem("Open", UI::HoveredButton->GetIndex(), 1),
						EditorDropdownItem("Rename", 800 + UI::HoveredButton->GetIndex(), 1),
						EditorDropdownItem("Copy", 1000 + UI::HoveredButton->GetIndex(), 1),
						EditorDropdownItem("Delete", 900 + UI::HoveredButton->GetIndex(), Vector3(1, 0.2, 0.2)),
					});
			}
			else if (Maths::IsPointIn2DBox(Vector2(-1, -0.5), Vector2(-0.7, 0.7), Input::MouseLocation))
			{
				MakeDropDownMenu(Input::MouseLocation, {
						EditorDropdownItem("cat_Create", 0, 1),
						EditorDropdownItem("Folder", 27, Vector3::Lerp(IconColors["dir"], Vector3(1), 0.8)),
						EditorDropdownItem("Material", 25, Vector3::Lerp(IconColors["jsmat"], Vector3(1), 0.8)),
						EditorDropdownItem("Material template", 26, Vector3::Lerp(IconColors["jsmtmp"], Vector3(1), 0.8)),
						EditorDropdownItem("Scene", 28, Vector3::Lerp(IconColors["jscn"], Vector3(1), 0.8)),
						EditorDropdownItem("Particle", 29, Vector3::Lerp(IconColors["jspart"], Vector3(1), 0.8)),
						EditorDropdownItem("Cubemap file", 30, Vector3::Lerp(IconColors["cbm"], Vector3(1), 0.8)),
						EditorDropdownItem("cat_Misc", 0, 1),
						EditorDropdownItem("Import file", 0, Vector3(0.5, 1, 0.5)),
					});
			}
		}
	}
	if (UserDraggingButton)
	{
		DragUI->SetPosition(Input::MouseLocation);
	}
	if (FPSUpdateTimer > 1)
	{
		TopBarText[0]->SetText("FPS: " + std::to_string(DisplayedFPS));
		DisplayedFPS = 0;
		FPSUpdateTimer = 0;
	}
	FPSUpdateTimer += Performance::DeltaTime;
	DisplayedFPS++;

	if (UI::HoveredButton)
	{
		SDL_SetCursor(GrabCursor);
	}
	else
	{
		SDL_SetCursor((Maths::IsPointIn2DBox(Vector2(0.7f, 0.675f), Vector2(-0.7f, -0.6f), Input::MouseLocation) && !SelectedTab)
			? CrossCursor : DefaultCursor);
	}
	if (ItemBrowserPath && !ItemBrowserPath->GetIsEdited())
	{
		ItemBrowserPath->SetText(ContentBrowser::CurrentFilePath);
	}
}

std::vector<EditorClassesItem> EditorUI::GetContentsOfCurrentCPPFolder()
{
	EditorClassesItem RootNode;
	RootNode.SubItems = CPPClasses;
	std::vector<EditorClassesItem> CurrentItems = RootNode.SubItems;
	for (const auto& path : CPPPath)
	{
		// Reorder the content so that folders are first and items are second.
		// It looks prettier in the item browser this way.
		std::vector<EditorClassesItem> ReordererdSubItems;
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (i.IsFolder) ReordererdSubItems.push_back(i);
		}
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (!i.IsFolder) ReordererdSubItems.push_back(i);
		}
		CurrentItems = ReordererdSubItems;
	}
	return CurrentItems;
}

std::string EditorUI::ToShortString(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << val;
	return stream.str();
}

std::string EditorUI::GetCurrentCPPPathString()
{
	std::string PathString = "C++";
	EditorClassesItem RootNode;
	RootNode.SubItems = CPPClasses;
	std::vector<EditorClassesItem> CurrentItems = RootNode.SubItems;
	for (const auto& path : CPPPath)
	{
		// Reorder the content so that folders are first and items are second.
		// It looks prettier in the item browser this way.
		std::vector<EditorClassesItem> ReordererdSubItems;
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (i.IsFolder) ReordererdSubItems.push_back(i);
		}
		for (const auto& i : CurrentItems[path].SubItems)
		{
			if (!i.IsFolder) ReordererdSubItems.push_back(i);
		}
		PathString.append("/" + CurrentItems[path].Name);
		CurrentItems = ReordererdSubItems;
	}
	PathString.append("/");
	return PathString;
}

void EditorUI::UpdateItemBrowser()
{
	LeftBoxes[1]->DeleteChildren();
	struct BrowserMode
	{
		std::string Name;
		unsigned int Index;
	};
	BrowserMode Modes[2] =
	{
		BrowserMode("Assets", 20),
		BrowserMode("Objects", 21)
	};
	for (unsigned int i = 0; i < 2; i++)
	{
		auto NewButton = new UIButton(true, 0, UIColors[0] * ((int)(ContentBrowserMode == (bool)i) + 1), this, Modes[i].Index);
		NewButton->SetBorder(UIBox::E_DARKENED_EDGE, 0.1);
		auto NewText = new UIText(0.5, UIColors[2], Modes[i].Name, &EngineUIText);
		NewText->SetPadding(0);
		NewButton->Align = UIBox::E_CENTERED;
		NewButton->AddChild(NewText);
		NewButton->SetPadding(0);
		NewButton->SetMinSize(Vector2(0.15, 0.05));
		LeftBoxes[1]->AddChild(NewButton);
	}
	RenamingContentAsset = false;
	ContentBrowser::UpdateContentAssets();
	Assets::ScanForAssets();
	auto ObjectsInFolder = GetContentsOfCurrentCPPFolder();

	LeftBoxes[2]->DeleteChildren();
	ItemBrowserButtons.clear();
	ItemBrowserTexts.clear();
	std::vector<UIBox*> NewBoxes;
	if (!ContentBrowserMode)
	{
		auto ContainerBox = new UIBackground(true, 0, UIColors[1]);
		auto BackButton = new UIButton(true, 0, 1, this, 5);
		BackButton->SetMinSize(Vector2(0.09, 0.16) / 2.f);
		BackButton->SetUseTexture(true, Textures[8]);
		BackButton->SetPadding(0.01);
		ContainerBox->AddChild(BackButton);
		ItemBrowserPath = new UITextField(true, 0, UIColors[1] * 2, this, 4, &EngineUIText);
		ItemBrowserPath->SetMinSize(Vector2(0.2, 0.08));
		ItemBrowserPath->SetText(ContentBrowser::CurrentFilePath);
		ItemBrowserPath->SetPadding(0.01);
		ItemBrowserPath->SetBorder(UIBox::E_ROUNDED, 0.5);
		ContainerBox->SetPadding(0);
		ContainerBox->AddChild(ItemBrowserPath);
		ContainerBox->SetMinSize(Vector2(0.3, 0.12));
		LeftBoxes[2]->AddChild(ContainerBox);
	}
	else
	{
		ItemBrowserPath = nullptr;
		auto ContainerBox = new UIBackground(true, 0, UIColors[1]);
		auto BackButton = new UIButton(true, 0, 1, this, 5);
		BackButton->SetMinSize(Vector2(0.09, 0.16) / 2.f);
		BackButton->SetUseTexture(true, Textures[8]);
		BackButton->SetPadding(0.01);
		ContainerBox->AddChild(BackButton);
		auto PathBox = new UIBox(false, 0);
		auto PathText = new UIText(0.5, 1, GetCurrentCPPPathString(), &EngineUIText);
		PathText->Wrap = true;
		PathText->WrapDistance = 0.25;
		PathBox->SetPadding(0, 0.025, 0, 0);
		PathText->SetPadding(0, 0, 0, 0);

		PathBox->AddChild(PathText);
		ContainerBox->AddChild(PathBox);
		ContainerBox->SetPadding(0);
		ContainerBox->SetMinSize(Vector2(0.3, 0.12));
		LeftBoxes[2]->AddChild(ContainerBox);
	}
	for (unsigned int i = 0; i < (!ContentBrowserMode ? (ContentBrowser::ContentAssets.size() / 3) : (ObjectsInFolder.size() / 2)) + 1; i++)
	{
		auto NewBox = new UIBox(true, 0);
		NewBox->SetPadding(0, 0, 0.01, 0);
		LeftBoxes[2]->AddChild(NewBox);
		NewBoxes.push_back(NewBox);
	}
	if (ContentBrowserMode)
	{
		for (unsigned int i = 0; i < ObjectsInFolder.size(); i++)
		{
			auto NewItemBrowserButton = new UIButton(false, 0, Vector3(UIColors[0] * 1.5), this, 200 + i);
			ItemBrowserButtons.push_back(NewItemBrowserButton);
			NewItemBrowserButton->SetMinSize(Vector2(0.1, 0.25) / 0.9f);
			NewItemBrowserButton->SetPadding(0.16 / 20.0f, 0.16 / 20.0f, 0.09 / 20.0f, 0.09 / 20.0f);
			NewItemBrowserButton->Align = UIBox::E_REVERSE;
			NewItemBrowserButton->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
			NewItemBrowserButton->SetNeedsToBeSelected(true);
			NewItemBrowserButton->SetCanBeDragged(true);
			NewItemBrowserButton->SetBorder(UIBox::E_ROUNDED, 0.8);
			auto ItemBrowserImage = new UIBackground(true, 0, 1);
			Vector3 IconColor = Vector3(1, 0.1, 0);
			auto FoundColor = IconColors.find(ObjectsInFolder[i].IsFolder ? "dir" : "cpp");
			std::string Name = ObjectsInFolder[i].Name;
			if (FoundColor != IconColors.end())
			{
				IconColor = FoundColor->second;
			}
			auto FoundNameColor = ObjectColors.find(Name);
			if (FoundNameColor != ObjectColors.end())
			{
				IconColor = FoundNameColor->second;
			}
			IconColor = Vector3::Lerp(IconColor, 0.5, 0.5);
			auto ItemBrowserImageBackground = new UIBackground(true, 0, IconColor);
			ItemBrowserImageBackground->SetPadding(0.16 / 15.0f, 0.16 / 20.0f, 0.09 / 15.0f, 0.09 / 15.0f);
			ItemBrowserImageBackground->AddChild(ItemBrowserImage);
			ItemBrowserImageBackground->SetBorder(UIBox::E_ROUNDED, 0.8);
			ItemBrowserImage->SetPadding(0);
			ItemBrowserImage->SetMinSize(Vector2(0.175));
			ItemBrowserImage->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
			ItemBrowserImage->SetUseTexture(true, ObjectsInFolder[i].IsFolder ? Textures[5] : Textures[0]);
			auto Text = new UIText(0.45, 1, ObjectsInFolder[i].Name, &EngineUIText);
			ItemBrowserTexts.push_back(Text);
			NewItemBrowserButton->AddChild(ItemBrowserImageBackground);
			NewItemBrowserButton->AddChild(Text);
			Text->SetPadding(0.001);
			Text->Wrap = true;
			Text->WrapDistance = 0.15;
			NewBoxes[i / 2]->AddChild(NewItemBrowserButton);
		}
	}
	else
	{
		if (!ContentBrowser::ContentAssets.size())
		{
			LeftBoxes[2]->AddChild(new UIText(0.45, 1, "Right click to create files", &EngineUIText));
		}
		for (unsigned int i = 0; i < ContentBrowser::ContentAssets.size(); i++)
		{
			auto NewItemBrowserButton = new UIButton(false, 0, Vector3(UIColors[0] * 1.5), this, 200 + i);
			ItemBrowserButtons.push_back(NewItemBrowserButton);
			NewItemBrowserButton->SetMinSize(Vector2(0.1, 0.25) / 1.2f);
			NewItemBrowserButton->SetPadding(0.16 / 20.0f, 0.16 / 20.0f, 0.09 / 20.0f, 0.09 / 20.0f);
			NewItemBrowserButton->Align = UIBox::E_REVERSE;
			NewItemBrowserButton->SetNeedsToBeSelected(true);
			NewItemBrowserButton->SetCanBeDragged(true);
			NewItemBrowserButton->SetBorder(UIBox::E_ROUNDED, 0.8);
			auto ItemBrowserImage = new UIBackground(true, 0, 1);
			auto Extension = GetExtension(ContentBrowser::ContentAssets[i].FilePath);
			if (std::filesystem::is_directory(ContentBrowser::ContentAssets[i].FilePath))
			{
				Extension = "dir";
			}
			Vector3 IconColor = Vector3(1, 0.1, 0);
			auto FoundColor = IconColors.find(Extension);
			if (FoundColor != IconColors.end())
			{
				IconColor = FoundColor->second;
			}
			auto ItemBrowserImageBackground = new UIBackground(true, 0, IconColor);
			ItemBrowserImageBackground->SetPadding(0.16 / 20.0f, 0.16 / 20.0f, 0.09 / 20.0f, 0.09 / 20.0f);
			ItemBrowserImageBackground->AddChild(ItemBrowserImage);
			ItemBrowserImageBackground->SetBorder(UIBox::E_ROUNDED, 0.8);
			ItemBrowserImage->SetPadding(0);
			ItemBrowserImage->SetMinSize(Vector2(0.09 / 1.2f, 0.16 / 1.2f));
			ItemBrowserImage->SetUseTexture(true, GetTextureFromFilePath(ContentBrowser::ContentAssets[i].FilePath, Textures));
			auto Text = new UIText(0.45, 1, GetFileNameWithoutExtensionFromPath(ContentBrowser::ContentAssets[i].FilePath), &EngineUIText);
			ItemBrowserTexts.push_back(Text);
			NewItemBrowserButton->AddChild(ItemBrowserImageBackground);
			NewItemBrowserButton->AddChild(Text);
			Text->SetPadding(0.001);
			Text->Wrap = true;
			Text->WrapDistance = 0.1;
			NewBoxes.at(i / 3)->AddChild(NewItemBrowserButton);
		}
	}
}

void EditorUI::UpdateLogMessages()
{
	if (LastConsoleLength != Log::Messages.size())
	{
		LogScrollObject->GetScrollObject()->Percentage = std::max(((int)Log::Messages.size() - 12) / 40.f, 0.f);
		LastConsoleLength = Log::Messages.size();
	}
	for (unsigned int i = 0; i < Log::Messages.size(); i++)
	{
		std::string Message = Log::Messages[i].Text;
		if (Log::Messages[i].Ammount) Message.append(" (x" + std::to_string(Log::Messages[i].Ammount + 1) + ")");
		EngineUIText.RenderText({ TextSegment(Message, Log::Messages[i].Color)}, Vector2(-0.6666, -0.625 - i / 40.0),
			0.9, 1, 1, 99, LogScrollObject->GetScrollObject());
	}
}

void EditorUI::UpdateContextMenu()
{
	RightBoxes[1]->DeleteChildren();
	SceneSettings.clear();
	SceneButtons.clear();
	SceneCategories.clear();
	if (SelectedObjects.size() > 0)
	{
		Properties.clear();
		WorldObject* SelectedObject = SelectedObjects[0];
		RightBoxes[1]->AddChild((new UIText(0.6, 1, "Object: " + SelectedObject->GetName(), &EngineUIText))->SetPadding(0.01));
		ContextMenu_GenerateSection(
			{
				ContextMenuSection(&SelectedObject->GetTransform().Location, Type::E_VECTOR3, "Location"),
				ContextMenuSection(&SelectedObject->GetTransform().Rotation, Type::E_VECTOR3, "Rotation"),
				ContextMenuSection(&SelectedObject->GetTransform().Scale, Type::E_VECTOR3, "Scale"),
				ContextMenuSection(&SelectedObject->Name, Type::E_STRING, "Name"),
			},
			"Object", SelectedObjects[0], 0);

		std::map<std::string, std::vector<ContextMenuSection>> Categories;

		for (Objects::Property i : SelectedObject->Properties)
		{
			auto Colon = i.Name.find_last_of(":");
			std::string CategoryName = SelectedObject->GetObjectDescription().Name;
			if (Colon != std::string::npos)
			{
				CategoryName = i.Name.substr(0, Colon);
				i.Name = i.Name.substr(Colon + 1);
			}
			if (!Categories.contains(CategoryName))
			{
				Categories.insert(std::pair(CategoryName, std::vector<ContextMenuSection>({ ContextMenuSection(i.Data, i.Type, i.Name) })));
			}
			else
			{
				Categories[CategoryName].push_back(ContextMenuSection(i.Data, i.Type, i.Name));
			}
		}
		char Iterator = 1;
		for (auto& i : Categories)
		{
			ContextMenu_GenerateSection(i.second, i.first, SelectedObjects[0], Iterator);
			Iterator++;
		}
	}
	else
	{
		RightBoxes[1]->AddChild((new UIText(0.6, 1, "Scene: " + GetFileNameWithoutExtensionFromPath(Scene::CurrentScene), &EngineUIText))->SetPadding(0.01));
		ContextMenu_GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.Direction, Type::E_VECTOR3, "Direction", true),
				ContextMenuSection(&Graphics::WorldSun.SunColor, Type::E_VECTOR3_COLOR, "Color"),
				ContextMenuSection(&Graphics::WorldSun.Intensity, Type::E_FLOAT, "Intensity")
			},
			"Sun", nullptr, 0);
		ContextMenu_GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldSun.AmbientColor, Type::E_VECTOR3_COLOR, "Color"),
				ContextMenuSection(&Graphics::WorldSun.AmbientIntensity, Type::E_FLOAT, "Intensity")
			},
			"Ambient light", nullptr, 1);
		ContextMenu_GenerateSection(
			{
				ContextMenuSection(&Graphics::WorldFog.FogColor, Type::E_VECTOR3_COLOR, "Color"),
				ContextMenuSection(&Graphics::WorldFog.Distance, Type::E_FLOAT, "Start distance"),
				ContextMenuSection(&Graphics::WorldFog.Falloff, Type::E_FLOAT, "Falloff"),
				ContextMenuSection(&Graphics::WorldFog.MaxDensity, Type::E_FLOAT, "Max density")
			},
			"Fog", nullptr, 2);
		ContextMenu_GenerateSection(
			{
				ContextMenuSection(&Graphics::MainFramebuffer->ReflectionCubemapName, Type::E_STRING, "Cubemap file"),
			},
			"Reflection", nullptr, 3);
	}
}

void EditorUI::UpdateTabs()
{
	/*
	* ---------------------------
	* | Co |               -----|
	* | lo | Tab Name      | X ||
	* | r  |               -----|
	* ---------------------------
	*/
	TabBox->DeleteChildren();
	for (int i = 0; i < EngineUIVariables::Tabs.size() * 2; i += 2)
	{
		auto NewTab = new UIButton(true, 0, UIColors[1] * ((SelectedTab == i / 2) ? 5 : 1.5), this, i - 200);
		NewTab->SetPadding(0, 0, 0, 0.02);
		auto TabColor = new UIBackground(true, 0, TabColors[EngineUIVariables::Tabs[i / 2].Type], Vector2(0.01, 0.05));
		TabColor->SetPadding(0);
		auto TabText = new UIText(0.5, 1, EngineUIVariables::Tabs[i / 2].Name, &EngineUIText);
		TabText->SetPadding(0.01);
		NewTab->AddChild(TabColor);
		NewTab->AddChild(TabText);
		NewTab->SetBorder(UIBox::E_DARKENED_EDGE, 0.2);
		TabBox->AddChild(NewTab);
		if (EngineUIVariables::Tabs[i / 2].CanBeClosed)
		{
			auto TabCloseButton = new UIButton(true, 0, 1, this, i - 199);
			TabCloseButton->SetUseTexture(true, Textures[4]);
			TabCloseButton->SetPadding(0);
			TabCloseButton->SetMinSize(Vector2(0.9 * 0.03, 1.6 * 0.03));
			NewTab->AddChild(TabCloseButton);
		}
	}
}

void EditorUI::OpenTab(int TabID, std::string File)
{
	if (TabItems[EngineUIVariables::Tabs[SelectedTab].Type])
	{
		TabItems[EngineUIVariables::Tabs[SelectedTab].Type]->Save();
	}
	EngineUIVariables::Tabs.push_back(EngineUIVariables::Tab(TabID, GetFileNameWithoutExtensionFromPath(File), File, true));
	SelectedTab = EngineUIVariables::Tabs.size() - 1;
	if (TabItems[EngineUIVariables::Tabs[SelectedTab].Type])
	{
		TabItems[EngineUIVariables::Tabs[SelectedTab].Type]->Load(EngineUIVariables::Tabs[SelectedTab].Path);
	}
	UpdateTabs();
}

UITextField* EditorUI::GenerateNewContextMenuTextField(std::string Content)
{
	auto NewElement = new UITextField(true, 0, 0.2, this, 44, &EngineUIText);
	((UITextField*)NewElement)->SetText(Content);
	((UITextField*)NewElement)->SetTextSize(0.4);
	NewElement->SetPadding(0.005, 0.005, 0.02, 0.005);
	NewElement->SetBorder(UIBox::E_ROUNDED, 0.5);
	NewElement->SetMinSize(Vector2(0.265, 0.04));
	NewElement->SetMaxSize(Vector2(0.3, 0.04));
	return NewElement;
}

void EditorUI::ContextMenu_GenerateSection(std::vector<ContextMenuSection> Section, std::string Name, WorldObject* ContextObject, unsigned int Index)
{
	auto SeperatorBorder = new UIButton(true, 0, 0.5, this, Index - 60);

	std::string Prefix = ContextObject ? "OBJ_CAT_" : "SCN_";

	auto SeperatorArrow = new UIBackground(true, Vector2(0), 0, Vector2(1, Graphics::AspectRatio) / 45);
	SeperatorArrow->SetPadding(0, 0, 0.01, 0);
	SeperatorArrow->SetUseTexture(true, CollapsedItems.contains(Prefix + Name) ? Textures[14] : Textures[13]);
	SeperatorArrow->SetTryFill(true);
	SeperatorBorder->AddChild(SeperatorArrow);

	auto SeperatorText = new UIText(0.5, 0, Name, &EngineUIText);
	SeperatorText->SetTryFill(true);
	SeperatorText->SetPadding(0.01);
	SeperatorBorder->SetPadding(0.03, 0.03, 0, 0);
	SeperatorBorder->SetMinSize(Vector2(0.3, 0));
	RightBoxes[1]->AddChild(SeperatorBorder);
	SeperatorBorder->AddChild(SeperatorText);

	SceneCategories.push_back(Name);
	if (CollapsedItems.contains(Prefix + Name))
	{
		return;
	}

	for (const auto& i : Section)
	{
		UIBox* NewElement = nullptr;
		UIText* NewElementText = new UIText(0.45, 1, i.Name, &EngineUIText);
		NewElementText->SetPadding(0.005, 0.005, 0.02, 0.005);
		RightBoxes[1]->AddChild(NewElementText);
		switch (i.Type)
		{
		// Vector3_Colors and Vector3s both use VectorFields, so we basically treat them the same
		case Type::E_VECTOR3_COLOR:
		case Type::E_VECTOR3:
			NewElement = new UIVectorField(0, *(Vector3*)i.Variable, this, 44, &EngineUIText);
			NewElement->SetPadding(0.005, 0, 0.02, 0);
			// Here we tell the VectorField to use RGB values instead of XYZ if required
			((UIVectorField*)NewElement)->SetValueType(i.Type == Type::E_VECTOR3 ? UIVectorField::E_XYZ : UIVectorField::E_RGB);
			break;
		case Type::E_FLOAT:
			NewElement = GenerateNewContextMenuTextField(ToShortString(*((float*)i.Variable)));
			break;
		case Type::E_INT:
			NewElement = GenerateNewContextMenuTextField(std::to_string(*((int*)i.Variable)));

			break;
		case Type::E_STRING:
			NewElement = GenerateNewContextMenuTextField(*((std::string*)i.Variable));
			break;
		case Type::E_BOOL:
			NewElement = new UIButton(true, 0, 0.75, this, 44);
			NewElement->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
			NewElement->SetMinSize(0.04);
			NewElement->SetBorder(UIBox::E_ROUNDED, 0.3);
			NewElement->SetPadding(0.01, 0.01, 0.02, 0.01);
			if (*((bool*)i.Variable))
			{
				((UIButton*)NewElement)->SetUseTexture(true, Textures[16]);
			}
			break;
		default:
			break;
		}
		if (NewElement)
		{
			RightBoxes[1]->AddChild(NewElement);
			SceneButtons.push_back(NewElement);
			SceneSettings.push_back(i);
		}
	}
}

void EditorUI::MakeDropDownMenu(Vector2 Position, std::vector<EditorDropdownItem> DropDownItems)
{
	DropDownMenuPosition = Position;
	if (DropDownMenu)
	{
		delete DropDownMenu;
	}
	DropDownMenu = new UIBox(false, Position - Vector2(0, DropDownItems.size() / 25.f));
	DropDownMenu->SetMinSize(Vector2(0.175, 0));
	DropDownMenu->Align = UIBox::E_REVERSE;
	unsigned int Index = 0;
	for (auto& i : DropDownItems)
	{
		bool IsCategory = i.Name.substr(0, 4) == "cat_";
		std::string Name = IsCategory ? i.Name.substr(4) : i.Name;
		UIBox* NewDropDownElement = nullptr;
		if (IsCategory)
		{
			NewDropDownElement = new UIBackground(true, 0, UIColors[1] * 3);
		}
		else
		{
			NewDropDownElement = new UIButton(true, 0, UIColors[1] * 3, this, i.Index);
		}
		auto DropDownText = new UIText(0.45, i.Color, Name, &EngineUIText);
		NewDropDownElement->SetPadding(0);
		NewDropDownElement->SetMinSize(Vector2(0.175, 0));
		DropDownText->SetPadding(0.005, 0.005, IsCategory ? 0.005 : 0.025, 0.005);
		DropDownText->Wrap = true;
		DropDownText->WrapDistance = 0.2;
		NewDropDownElement->SetTryFill(true);
		NewDropDownElement->AddChild(DropDownText);
		DropDownMenu->AddChild(NewDropDownElement);
	}
}

void EditorUI::UpdateObjectList()
{
	unsigned int i = 0;
	RightBoxes[0]->DeleteChildren();
	auto ObjectList = GetObjectList();
	GenerateObjectListSection(ObjectList, 0);
}

void EditorUI::GenUITextures()
{
	const int ImageSize = 20;
	std::string Images[ImageSize]
	{												//Texture Indices
		"EditorContent/Images/CPPClass.png",		//00 -> C++ class icon
		"EditorContent/Images/Wireframe.png",		//01 -> Symbol for button to toggle wireframe
		"EditorContent/Images/Save.png",			//02 -> Save Button
		"EditorContent/Images/Build.png",			//03 -> Package button
		"EditorContent/Images/X.png",				//04 -> X Symbol
		"EditorContent/Images/Folder.png",			//05 -> Folder symbol for item browser
		"EditorContent/Images/Sound.png",			//06 -> Sound symbol for item browser
		"EditorContent/Images/Scene.png",			//07 -> Scene symbol for item browser
		"EditorContent/Images/ExitFolder.png",		//08 -> Icon used to navigate back one folder
		"EditorContent/Images/Material.png",		//09 -> Material symbol for item browser
		"EditorContent/Images/MaterialTemplate.png",//10 -> Material Template symbol for item browser
		"EditorContent/Images/Model.png",			//11 -> Model symbol for item browser
		"EditorContent/Images/Reload.png",			//12 -> Reload symbol
		"EditorContent/Images/ExpandedArrow.png",	//13 -> Expanded arrow
		"EditorContent/Images/CollapsedArrow.png",	//14 -> Collapsed arrow
		"EditorContent/Images/Preferences.png",		//15 -> Collapsed arrow
		"EditorContent/Images/Checkbox.png",		//16 -> Checked checkbox
		"EditorContent/Images/Cubemap.png",			//17 -> Cubemap icon
		"EditorContent/Images/Texture.png",			//18 -> Texture icon
	};

	for (int i = 0; i < Textures.size(); i++)
	{
		glDeleteTextures(1, &Textures.at(i));
	}
	for (int i = 0; i < ImageSize; i++)
	{
		int TextureWidth = 0;
		int TextureHeigth = 0;
		int BitsPerPixel = 0;
		stbi_set_flip_vertically_on_load(true);
		auto TextureBuffer = stbi_load(Images[i].c_str(), &TextureWidth, &TextureHeigth, &BitsPerPixel, 4);


		GLuint TextureID;
		glGenTextures(1, &TextureID);
		glBindTexture(GL_TEXTURE_2D, TextureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TextureWidth, TextureHeigth, 0, GL_RGBA, GL_UNSIGNED_BYTE, TextureBuffer);

		Textures.push_back(TextureID);
		if (TextureBuffer)
		{
			stbi_image_free(TextureBuffer);
		}
	}
}

std::vector<EditorUI::ObjectListItem> EditorUI::GetObjectList()
{
	std::vector<ObjectListItem> ObjectList;
	ObjectCategories.clear();
	size_t ListIndex = 0;
	for (WorldObject* o : Objects::AllObjects)
	{
		ObjectListItem* SceneList = nullptr;
		// Get the list for the scene the object belongs to
		for (auto& item : ObjectList)
		{
			if (item.Name == GetFileNameFromPath(o->CurrentScene))
			{
				SceneList = &item;
			}
		}

		if (!SceneList)
		{
			std::string SceneName = GetFileNameFromPath(o->CurrentScene);
			ObjectList.push_back(ObjectListItem(SceneName, {}, true, CollapsedItems.contains("OBJ_CAT_" + SceneName)));
			ObjectCategories.push_back(GetFileNameFromPath(o->CurrentScene));
			ObjectList[ObjectList.size() - 1].ListIndex = ObjectCategories.size() - 1;
			SceneList = &ObjectList[ObjectList.size() - 1];
		}

		// Seperate the Object's category into multiple strings
		std::string CurrentPath = Objects::GetCategoryFromID(o->GetObjectDescription().ID);
		std::vector<std::string> PathElements;
		size_t Index = CurrentPath.find_first_of("/");
		while (Index != std::string::npos)
		{
			Index = CurrentPath.find_first_of("/");
			PathElements.push_back(CurrentPath.substr(0, Index));
			CurrentPath = CurrentPath.substr(Index + 1);
			Index = CurrentPath.find_first_of("/");
		}
		PathElements.push_back(CurrentPath);

		ObjectListItem* CurrentList = SceneList;
		if (SceneList->IsCollapsed) continue;
		for (const auto& elem : PathElements)
		{
			ObjectListItem* NewList = nullptr;
			for (auto& c : CurrentList->Children)
			{
				if (c.Name != elem) continue;
				NewList = &c;
				break;
			}

			if (!NewList && !CurrentList->IsCollapsed)
			{
				int it = 0;
				while (true)
				{
					if (it >= (int)CurrentList->Children.size() || CurrentList->Children[it].Object)
					{
						break;
					}
					it++;
				}
				ObjectCategories.push_back(elem);
				CurrentList->Children.insert(CurrentList->Children.begin() + it, ObjectListItem(elem, {}, false, CollapsedItems.contains("OBJ_CAT_" + elem)));
				CurrentList->Children[it].ListIndex = ObjectCategories.size() - 1;
				NewList = &CurrentList->Children[it];
			}

			CurrentList = NewList;
		}
		if (CurrentList && !CurrentList->IsCollapsed)
		{
			CurrentList->Children.push_back(ObjectListItem(o, ListIndex));
		}
		ListIndex++;
	}
	return ObjectList;
}

void EditorUI::GenerateObjectListSection(std::vector<ObjectListItem> Section, float Depth)
{
	for (auto& Object : Section)
	{
		/*
		* ----------------------
		* | Co |               |
		* | lo | Object Name   |
		* | r  |               |
		* ----------------------
		*/

		auto NewListEntryBackground = new UIButton(true, 0, UIColors[1], this, Object.Object ? 300 + Object.ListIndex : 1300 + Object.ListIndex);
		NewListEntryBackground->SetBorder(UIBox::E_ROUNDED, 0.8);
		if (SelectedObjects.size() && Object.Object == SelectedObjects[0])
		{
			NewListEntryBackground->SetColor(UIColors[1] * 2);
		}
		NewListEntryBackground->SetMinSize(Vector2(0.2, 0));
		NewListEntryBackground->SetPadding(0.01, 0.0, 0.01 + Depth, 0.01);
		auto ListEntryObjectColor = new UIBackground(true, 0, Vector3(0.5, 0.5, 0.5), Vector2(0.01, 0));
		ListEntryObjectColor->SetTryFill(true);
		ListEntryObjectColor->SetPadding(0);
		NewListEntryBackground->AddChild(ListEntryObjectColor);
		auto TextBox = new UIBox(false, 0);

		if (!Object.Object)
		{
			auto CollapsedArrow = new UIBackground(true, Vector2(0), 1, Vector2(1, Graphics::AspectRatio) / 45);
			CollapsedArrow->SetPadding(0, 0, 0.01, 0);
			CollapsedArrow->SetUseTexture(true, Object.IsCollapsed ? Textures[14] : Textures[13]);
			CollapsedArrow->SetTryFill(true);
			NewListEntryBackground->AddChild(CollapsedArrow);
		}

		TextBox->SetPadding(0);
		NewListEntryBackground->AddChild(TextBox);
		UIText* ListEntryText;
		if (Object.Object)
		{
			Vector3 IconColor;
			auto FoundColor = ObjectColors.find(Object.Object->GetObjectDescription().Name);
			if (FoundColor != ObjectColors.end())
			{
				IconColor = FoundColor->second;
			}
			else
			{
				IconColor = ObjectColors["None"];
			}
			ListEntryObjectColor->SetColor(IconColor);
			ListEntryText = new UIText(0.3, 0.7, "Class " + Object.Object->GetObjectDescription().Name, &EngineUIText);
		}
		else
		{
			ListEntryObjectColor->SetColor(IconColors["dir"]);
			ListEntryText = new UIText(0.3, 0.7, Object.IsScene ? "Scene" : "Category", &EngineUIText);
		}
		ListEntryText->SetPadding(0.0, 0.01, 0.01, 0.01);
		ListEntryText->Wrap = true;
		ListEntryText->WrapDistance = 0.3;
		TextBox->AddChild(ListEntryText);
		ListEntryText = new UIText(0.4, 1, Object.Name, &EngineUIText);
		ListEntryText->SetPadding(0.0, 0, 0.01, 0.01);
		ListEntryText->Wrap = true;
		ListEntryText->WrapDistance = 0.5;
		TextBox->AddChild(ListEntryText);
		RightBoxes[0]->AddChild(NewListEntryBackground);
		if (!Object.Object)
		{
			GenerateObjectListSection(Object.Children, Depth + 0.01);
		}
	}
}

std::vector<EditorClassesItem> EditorUI::GetEditorUIClasses()
{
	std::vector<std::string> IDs;
	EditorClassesItem RootPath;
	for (const auto& Object : Objects::EditorObjects)
	{
		// First seperate the Category into multiple names. For example: "Default/Rendering" -> { "Default", "Rendering" }
		std::string CurrentPath = Objects::GetCategoryFromID(Object.ID);
		EditorClassesItem* CurrentParent = &RootPath;
		if (CurrentPath.empty())
		{
			EditorClassesItem NewItem;
			NewItem.Name = Object.Name;
			NewItem.Object = Object;
			CurrentParent->SubItems.push_back(NewItem);
		}
		std::vector<std::string> PathElements;
		size_t Index = CurrentPath.find_first_of("/");


		while (Index != std::string::npos)
		{
			Index = CurrentPath.find_first_of("/");
			PathElements.push_back(CurrentPath.substr(0, Index));
			CurrentPath = CurrentPath.substr(Index + 1);
			Index = CurrentPath.find_first_of("/");
		}
		PathElements.push_back(CurrentPath);

		// Iterate through every 'element' we just got from the Category string
		for (const auto& elem : PathElements)
		{
			bool Found = false;
			// If that element already exists, we continue inside of it.
			for (size_t i = 0; i < CurrentParent->SubItems.size(); i++)
			{
				if (elem == CurrentParent->SubItems[i].Name)
				{
					CurrentParent = &CurrentParent->SubItems[i];
					Found = true;
					break;
				}
			}
			if (Found) continue;
			// Else we create that new element.
			EditorClassesItem NewPath;
			NewPath.IsFolder = true;
			NewPath.Name = elem;
			CurrentParent->SubItems.push_back(NewPath);
			CurrentParent = &CurrentParent->SubItems[CurrentParent->SubItems.size() - 1];
		}
		// Create a new item structure so we can add it to the folder "file system"
		EditorClassesItem NewItem;
		NewItem.Name = Object.Name;
		NewItem.Object = Object;
		CurrentParent->SubItems.push_back(NewItem);
	}

	// Debug view to display 'folder structure'
	// its very basic and only goes 3 levels deep
	// 
	/*for (const auto& i : RootPath.SubItems)
	{
		Log::Print(i.Name);
		for (const auto& j : i.SubItems)
		{
			Log::Print("   " + j.Name);
			for (const auto& k : j.SubItems)
			{
				Log::Print("      " + k.Name);
			}
		}
	}*/
	return RootPath.SubItems;
}

#endif