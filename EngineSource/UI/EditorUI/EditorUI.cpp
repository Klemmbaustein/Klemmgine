﻿#ifdef EDITOR
#include "EditorUI.h"
#include "Utility/stb_image.h"
#include "Engine/FileUtility.h"
#include <filesystem>
#include "Math/Math.h"
#include "Math/Collision/Collision.h"
#include "Engine/Scene.h"
#include <Engine/EngineProperties.h>
#include <UI/UIScrollBox.h>	
#include <UI/EditorUI/UIVectorField.h>
#include <UI/EditorUI/LogUI.h>
#include <UI/EditorUI/Toolbar.h>
#include <UI/EditorUI/ItemBrowser.h>
#include <UI/EditorUI/StatusBar.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/EditorUI/ObjectList.h>
#include <UI/EditorUI/ContextMenu.h>
#include <GL/glew.h>
#include <atomic>
#include <Engine/Log.h>
#include <Engine/Input.h>
#include <UI/EditorUI/Popups/DialogBox.h>
#include <Engine/Console.h>
#include <CSharp/CSharpInterop.h>

namespace Editor
{
	EditorUI* CurrentUI = nullptr;
	bool DraggingTab = false;
	bool TabDragHorizontal = false;
	bool DraggingPopup = false;
	bool HoveringPopup = false;
	bool PrevHoveringPopup = false;
	Vector2 DragMinMax;
	Vector2 NewDragMinMax = DragMinMax;
	bool IsSavingScene = false;

	Vector3 NewUIColors[EditorUI::NumUIColors] =
	{
		Vector3(0.85, 0.85, 0.85),	//Default background
		Vector3(0.6),				//Dark background
		Vector3(0),					//Highlight color,
		Vector3(1)
	};

	bool LightMode = false;

	Vector3 ReplaceWithNewUIColor(Vector3 PrevColor)
	{
		for (uint8_t i = 0; i < EditorUI::NumUIColors; i++)
		{
			if (Editor::CurrentUI->UIColors[i] == PrevColor)
			{
				return Editor::NewUIColors[i];
			}
		}

		return PrevColor;
	}
	std::atomic<bool> CanHotreload = false;
}

namespace UI
{
	extern std::vector<UIBox*> UIElements;
}

bool ChangedScene = false;


#ifdef ENGINE_CSHARP
std::string EditorUI::LaunchInEditorArgs;

void EditorUI::LaunchInEditor()
{
	std::string ProjectName = Build::GetProjectBuildName();
	try
	{
		if (!std::filesystem::exists("x64/Debug/" + ProjectName + ".exe")
			|| std::filesystem::last_write_time("x64/Debug/" + ProjectName + ".exe") < std::filesystem::last_write_time("Code"))
		{
			Log::Print("Detected uncompiled changes to C++ code. Rebuilding...", Log::LogColor::Yellow);
			Build::BuildCurrentSolution("Debug");
		}
		if (!std::filesystem::exists("CSharp/Build/CSharpAssembly.dll") 
			|| std::filesystem::last_write_time("CSharp/Build/CSharpAssembly.dll") < std::filesystem::last_write_time("Scripts"))
		{
			RebuildAndHotReload();
		}
	}
	catch (std::exception& e)
	{
	}
	system(("x64\\Debug\\" + ProjectName + ".exe " + LaunchInEditorArgs).c_str());
}
void EditorUI::RebuildAndHotReload()
{
	Log::Print("Rebuilding C# assembly...", Log::LogColor::Green);
	system("cd Scripts && dotnet build");
	Editor::CanHotreload = true;
}
#endif

void EditorUI::SaveCurrentScene()
{
	if (Scene::CurrentScene.empty())
	{
		Log::Print("Saving scene \"Untitled\"", Vector3(0.3, 0.4, 1));
		Scene::SaveSceneAs("Content/Untitled");
	}
	else
	{
		Log::Print("Saving scene \"" + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene) + "\"", Vector3(0.3, 0.4, 1));
		Scene::SaveSceneAs(Scene::CurrentScene);
	}
	ChangedScene = false;
}

std::string EditorSceneToOpen;
void EditorUI::OpenScene(std::string NewScene)
{
	if (ChangedScene)
	{
		EditorSceneToOpen = NewScene;
		new DialogBox(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene), 0, "Scene has unsaved changes. Save?",
			{
				DialogBox::Answer("Yes", []() {SaveCurrentScene(); ChangedScene = false; Scene::LoadNewScene(EditorSceneToOpen);
				Viewport::ViewportInstance->ClearSelectedObjects();
				Scene::Tick();
				Editor::CurrentUI->UIElements[5]->UpdateLayout();
				Editor::CurrentUI->UIElements[6]->UpdateLayout(); }),

				DialogBox::Answer("No", []() {ChangedScene = false; Scene::LoadNewScene(EditorSceneToOpen);
				Viewport::ViewportInstance->ClearSelectedObjects();
				Scene::Tick();
				Editor::CurrentUI->UIElements[5]->UpdateLayout();
				Editor::CurrentUI->UIElements[6]->UpdateLayout(); }),

				DialogBox::Answer("Cancel", nullptr)
			});
		return;
	}
	Viewport::ViewportInstance->ClearSelectedObjects();
	ChangedScene = false; 
	Scene::LoadNewScene(NewScene);
	Scene::Tick();
	Editor::CurrentUI->UIElements[5]->UpdateLayout();
	Editor::CurrentUI->UIElements[6]->UpdateLayout();
}

bool EditorUI::GetUseLightMode()
{
	return Editor::LightMode;
}

void EditorUI::SetUseLightMode(bool NewLightMode)
{
	if (NewLightMode != Editor::LightMode)
	{
		for (auto i : UI::UIElements)
		{
			if (dynamic_cast<UIText*>(i))
			{
				dynamic_cast<UIText*>(i)->SetColor(Editor::ReplaceWithNewUIColor(dynamic_cast<UIText*>(i)->GetColor()));
			}
			if (dynamic_cast<UIButton*>(i))
			{
				dynamic_cast<UIButton*>(i)->SetColor(Editor::ReplaceWithNewUIColor(dynamic_cast<UIButton*>(i)->GetColor()));
			}
			if (dynamic_cast<UIBackground*>(i))
			{
				dynamic_cast<UIBackground*>(i)->SetColor(Editor::ReplaceWithNewUIColor(dynamic_cast<UIBackground*>(i)->GetColor()));
			}
			if (dynamic_cast<UITextField*>(i))
			{
				dynamic_cast<UITextField*>(i)->SetColor(Editor::ReplaceWithNewUIColor(dynamic_cast<UITextField*>(i)->GetTextColor()));
				dynamic_cast<UITextField*>(i)->SetTextColor(Editor::ReplaceWithNewUIColor(dynamic_cast<UITextField*>(i)->GetTextColor()));
			}
		}
		std::swap(Editor::CurrentUI->UIColors, Editor::NewUIColors);
		Editor::LightMode = NewLightMode;
		Log::Print("Toggled light mode", Log::LogColor::Yellow);
	}
	UIBox::ForceUpdateUI();
}

void EditorUI::CreateFile(std::string Path, std::string Name, std::string Ext)
{
	std::string Addition;
	size_t AdditionNum = 0;
	while (std::filesystem::exists(Path + "/" + Name + Addition + "." + Ext))
	{
		Addition = "_" + std::to_string(++AdditionNum);
	}

	std::ofstream out = std::ofstream(Path + "/" + Name + Addition + "." + Ext);
	out.close();
}

EditorUI::EditorUI()
{
	Editor::CurrentUI = this;

	GenUITextures();

	Cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	Cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	Cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	Cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	Cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	Cursors[6] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	//new UIBackground(true, -1, 0.5, 2);

	UIElements[0] = new StatusBar(UIColors);
	UIElements[1] = new LogUI(UIColors, Vector2(-0.7, -1), Vector2(1.4, 0.4));
	UIElements[2] = new Toolbar(UIColors, Vector2(-0.7, 0.73), Vector2(1.4, 0.22));
	UIElements[3] = new ItemBrowser(UIColors, Vector2(-1, -1), Vector2(0.3, 1.95));
	UIElements[4] = new Viewport(UIColors, Vector2(-0.7, -0.6), Vector2(1.4, 1.33));
	UIElements[5] = new ObjectList(UIColors, Vector2(0.7, -0.2), Vector2(0.3, 1.15));
	UIElements[6] = new ContextMenu(UIColors, Vector2(0.7, -1), Vector2(0.3, 0.8));

	// Load preferences from the preference tab after the UI is finished setting up
	Viewport::ViewportInstance->TabInstances[6]->Load("");

	Console::RegisterCommand(Console::Command("build", []() {new std::thread(Build::TryBuildProject, "Build/"); }, {}));
	Console::RegisterCommand(Console::Command("save", EditorUI::SaveCurrentScene, {}));
#ifdef ENGINE_CSHARP
	Console::RegisterCommand(Console::Command("reload", EditorUI::RebuildAndHotReload, {}));
	Console::RegisterCommand(Console::Command("run", EditorUI::LaunchInEditor, {}));
#endif
}

void(*QuitFunction)();

void EditorUI::OnLeave(void(*ReturnF)())
{
	QuitFunction = ReturnF;
	if (ChangedScene)
	{
		new DialogBox("Scene", 0,
			"Save changes to scene before quitting?",
			{
				DialogBox::Answer("Yes", []() {
				SaveCurrentScene();
				QuitFunction(); }),
				DialogBox::Answer("No", ReturnF),
				DialogBox::Answer("Cancel", nullptr)
			});
	}
	else
	{
		ReturnF();
	}
}

void EditorUI::Tick()
{
#ifdef ENGINE_CSHARP
	if (Editor::CanHotreload == true)
	{
		Log::Print("Finished building assembly. Hotreloading .dll file...", Log::LogColor::Yellow);
		CSharp::ReloadCSharpAssembly();
		Editor::CanHotreload = false;
	}
#endif

	Editor::HoveringPopup = Editor::PrevHoveringPopup;
	Editor::PrevHoveringPopup = false;
	Editor::DraggingPopup = false;
	if (!Editor::DraggingTab)
	{
		Editor::NewDragMinMax = Vector2(-1, 1);
	}
	Editor::DragMinMax = Editor::NewDragMinMax;

	if (UI::HoveredButton)
	{
		CurrentCursor = E_GRAB;
	}
	if (CurrentCursor < E_LAST_CURSOR && CurrentCursor >= 0)
	{
		SDL_SetCursor(Cursors[CurrentCursor]);
	}
	CurrentCursor = Editor::DraggingTab ? (Editor::TabDragHorizontal ? E_RESIZE_WE : E_RESIZE_NS) : E_DEFAULT;

	if (Input::IsLMBDown && Dropdown && !Dropdown->IsHovered())
	{
		delete Dropdown;
		Dropdown = nullptr;
	}

	if (DraggedItem)
	{
		DraggedItem->SetPosition(Input::MouseLocation - DraggedItem->GetUsedSize() / 2);
		if (!Input::IsLMBDown)
		{
			delete DraggedItem;
			DraggedItem = nullptr;
		}
	}

	if (Input::IsKeyDown(SDLK_LCTRL) && Input::IsKeyDown(SDLK_s))
	{
		if (!Input::IsRMBDown && ChangedScene && !Editor::IsSavingScene)
		{
			SaveCurrentScene();
		}
		Editor::IsSavingScene = true;
	}
	else
	{
		Editor::IsSavingScene = false;
	}
}



void EditorUI::ShowDropdownMenu(std::vector<DropdownItem> Menu, Vector2 Position)
{
	if (Dropdown)
	{
		delete Dropdown;
	}

	CurrentDropdown = Menu;
	Dropdown = new UIBackground(false, Position, UIColors[0] * 2);
	Dropdown->Align = UIBox::E_REVERSE;
	Dropdown->SetBorder(UIBox::E_ROUNDED, 0.4);
	for (size_t i = 0; i < Menu.size(); i++)
	{
		UIBox* NewElement = nullptr;
		if (Menu[i].Title[0] == '#')
		{
			NewElement = new UIBackground(true, 0, UIColors[0] * 2);
			Menu[i].Title = " " + Menu[i].Title.substr(Menu[i].Title.find_first_not_of("# "));
		}
		else
		{
			NewElement = new UIButton(true, 0, UIColors[0] * 2, this, i);
			Menu[i].Title = "   " + Menu[i].Title;
		}
		NewElement->SetBorder(UIBox::E_ROUNDED, 0.4);
		NewElement->SetMinSize(Vector2(0.16, 0));
		NewElement->SetPadding(0);
		NewElement->AddChild((new UIText(0.45, UIColors[2], Menu[i].Title, EngineUIText))
			->SetPadding(0));
		Dropdown->AddChild(NewElement);
	}

	UIBox::DrawAllUIElements();
	Dropdown->SetPosition(Dropdown->GetPosition() - Vector2(0, Dropdown->GetUsedSize().Y));
}

std::string EditorUI::ToShortString(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << val;
	return stream.str();
}


void EditorUI::GenUITextures()
{
	const int ImageSize = 23;
	std::string Images[ImageSize]
	{								//Texture Indices
		"CPPClass.png",				//00 -> C++ class icon
		"Wireframe.png",			//01 -> Symbol for button to toggle wireframe
		"Save.png",					//02 -> Save Button
		"Build.png",				//03 -> Package button
		"X.png",					//04 -> X Symbol
		"Folder.png",				//05 -> Folder symbol for item browser
		"Sound.png",				//06 -> Sound symbol for item browser
		"Scene.png",				//07 -> Scene symbol for item browser
		"ExitFolder.png",			//08 -> Icon used to navigate back one folder
		"Material.png",				//09 -> Material symbol for item browser
		"MaterialTemplate.png",		//10 -> Material Template symbol for item browser
		"Model.png",				//11 -> Model symbol for item browser
		"Reload.png",				//12 -> Reload symbol
		"ExpandedArrow.png",		//13 -> Expanded arrow
		"CollapsedArrow.png",		//14 -> Collapsed arrow
		"Preferences.png",			//15 -> Collapsed arrow
		"Checkbox.png",				//16 -> Checked checkbox
		"Cubemap.png",				//17 -> Cubemap icon
		"Texture.png",				//18 -> Texture icon
		"Particle.png",				//19 -> Particle icon
		"Settings.png",				//20 -> Settings icon
		"Play.png",					//21 -> Play icon
		"CSharpClass.png",			//22 -> CSharp class icon
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
		auto TextureBuffer = stbi_load(("../../EditorContent/Images/" + Images[i]).c_str(), &TextureWidth, &TextureHeigth, &BitsPerPixel, 4);


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
			if (item.Name == FileUtil::GetFileNameFromPath(o->CurrentScene))
			{
				SceneList = &item;
			}
		}

		if (!SceneList)
		{
			std::string SceneName = FileUtil::GetFileNameFromPath(o->CurrentScene);
			ObjectList.push_back(ObjectListItem(SceneName, {}, true, CollapsedItems.contains("OBJ_CAT_" + SceneName)));
			ObjectCategories.push_back(FileUtil::GetFileNameFromPath(o->CurrentScene));
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
		if (!CurrentPath.empty())
		{
			PathElements.push_back(CurrentPath);
		}

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

void EditorUI::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		if (CurrentDropdown[Index].OnPressed)
		{
			CurrentDropdown[Index].OnPressed();
		}
		delete Dropdown;
		Dropdown = nullptr;
	}
}

void EditorUI::OnResized()
{
	for (auto i : UIElements)
	{
		i->UpdateLayout();
	}
}

#endif