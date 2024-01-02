#ifdef EDITOR
#include "EditorUI.h"
#include "Engine/Utility/FileUtility.h"
#include <Objects/CSharpObject.h>
#include <filesystem>
#include "Math/Math.h"
#include "Math/Collision/Collision.h"
#include "Engine/Scene.h"
#include <Engine/EngineProperties.h>
#include <UI/UIScrollBox.h>	
#include <UI/EditorUI/UIVectorField.h>
#include <UI/EditorUI/LogUI.h>
#include <UI/EditorUI/Toolbar.h>
#include <UI/EditorUI/AssetBrowser.h>
#include <UI/EditorUI/ClassesBrowser.h>
#include <UI/EditorUI/StatusBar.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/EditorUI/ObjectList.h>
#include <UI/EditorUI/ContextMenu.h>
#include <atomic>
#include <thread>
#include <Engine/Log.h>
#include <Engine/BackgroundTask.h>
#include <Engine/Input.h>
#include <UI/EditorUI/Popups/DialogBox.h>
#include <Engine/Console.h>
#include <CSharp/CSharpInterop.h>
#include <Rendering/Utility/BakedLighting.h>
#include <Engine/File/Assets.h>
#include <UI/UIDropdown.h>
#include <SDL.h>
#include <Rendering/Texture/Texture.h>
#include <Engine/Application.h>
#include <UI/EditorUI/SettingsPanel.h>
#include <Networking/Networking.h>

int EditorUI::NumLaunchClients = 1;
TextRenderer* EditorUI::Text = nullptr;
TextRenderer* EditorUI::MonoText = nullptr;
EditorPanel* EditorUI::RootPanel = nullptr;
std::vector<unsigned int> EditorUI::Textures;
std::vector<WorldObject*> EditorUI::SelectedObjects;
EditorUI::CursorType EditorUI::CurrentCursor = CursorType::Default;

Vector3 EditorUI::UIColors[NumUIColors] =
{
	Vector3(0.125f, 0.125f, 0.13f),	// Default background
	Vector3(0.08f),					// Dark background
	Vector3(1),						// Highlight color
	Vector3(0.2f),					// Brighter background
};

namespace Editor
{
	Vector2 DragMinMax;
	Vector2 NewDragMinMax = DragMinMax;
	bool IsSavingScene = false;
	bool IsBakingScene = false;
	bool LaunchCurrentScene = true;
	bool SaveSceneOnLaunch = false;

	Vector3 NewUIColors[EditorUI::NumUIColors] =
	{
		Vector3(0.85f, 0.85f, 0.85f),	//Default background
		Vector3(0.6f),				//Dark background
		Vector3(0),					//Highlight color,
		Vector3(1)
	};

	bool LightMode = false;

	static Vector3 ReplaceWithNewUIColor(Vector3 PrevColor)
	{
		for (uint8_t i = 0; i < EditorUI::NumUIColors; i++)
		{
			if (Application::EditorInstance->UIColors[i] == PrevColor)
			{
				return Editor::NewUIColors[i];
			}
		}

		return PrevColor;
	}
	std::atomic<bool> CanHotreload = false;

	struct ProcessInfo
	{
		std::string Command;
		FILE* Pipe = nullptr;
		std::thread* Thread = nullptr;
		std::atomic<bool> Active = false;
		bool Async = false;
	};

	std::map<FILE*, ProcessInfo> ProcessPipes;

	static void ReadProcessPipe(FILE* p)
	{
		std::string CurrentMessage;
		while (!feof(p))
		{
			char NewChar = (char)fgetc(p);

			if (NewChar == '\n')
			{
				Log::Print(CurrentMessage);
				CurrentMessage.clear();
			}
			else
			{
				CurrentMessage.append({ NewChar });
			}
		}
		fclose(p);
		ProcessPipes[p].Active = false;
	}
}

namespace UI
{
	extern std::vector<UIBox*> UIElements;
}

bool ChangedScene = false;

std::string EditorUI::LaunchInEditorArgs;
bool EditorUI::LaunchWithServer = false;
static std::string LaunchCommandLine;
void EditorUI::LaunchInEditor()
{
	std::string ProjectName = Build::GetProjectBuildName();
	try
	{
#if !ENGINE_NO_SOURCE && !__linux__
		if (!std::filesystem::exists("bin/" + ProjectName + "-Debug.exe")
			|| std::filesystem::last_write_time("bin/" + ProjectName + "-Debug.exe") < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" }))
		{
			Log::Print("Detected uncompiled changes to C++ code. Rebuilding...", Log::LogColor::Yellow);
			Build::BuildCurrentSolution("Debug");
		}

		if (Project::UseNetworkFunctions && (!std::filesystem::exists("bin/" + ProjectName + "-Server.exe")
			|| std::filesystem::last_write_time("bin/" + ProjectName + "-Server.exe") < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" })))
		{
			Build::BuildCurrentSolution("Server");
		}
#endif

		if ((!std::filesystem::exists("CSharp/Build/CSharpAssembly.dll")
			|| std::filesystem::last_write_time("CSharp/Build/CSharpAssembly.dll") < FileUtil::GetLastWriteTimeOfFolder("Scripts", { "obj" }))
			&& CSharp::GetUseCSharp())
		{
			RebuildAssembly();
		}
	}
	catch (std::exception& e)
	{
		Log::Print("Exception thrown when trying to check for rebuild. " + std::string(e.what()));
		return;
	}

	if (Editor::SaveSceneOnLaunch)
	{
		SaveCurrentScene();
	}

	std::string Args = LaunchInEditorArgs;
	if (Editor::LaunchCurrentScene)
	{
		Args.append(" -scene " + FileUtil::GetFileNameFromPath(Scene::CurrentScene));
	}

#if ENGINE_NO_SOURCE || __linux__
	ProjectName = "Klemmgine";
#endif
#if _WIN32

	std::string CommandLine = "bin\\" + ProjectName + "-Debug.exe -nostartupinfo -editorPath " + Application::GetEditorPath() + " " + Args;
#else
	std::string CommandLine = "./bin/" + ProjectName + "-Debug -nostartupinfo -editorPath " + Application::GetEditorPath() + " " + Args;
#endif
	if (LaunchWithServer)
	{
		CommandLine.append(" -connect localhost ");
}

	Log::Print("[Debug]: Starting process: " + CommandLine, Log::LogColor::Blue);
#if _WIN32
	std::string Command;
	for (int i = 0; i < NumLaunchClients; i++)
	{
		Command.append("start /b " + CommandLine);
		if (i < NumLaunchClients - 1)
		{
			Command.append(" && ");
		}
	}
	int ret = system(Command.c_str());
#else
	for (int i = 0; i < NumLaunchClients; i++)
	{
		LaunchCommandLine = CommandLine;
		new BackgroundTask([]() { system((LaunchCommandLine).c_str()); });
	}
#endif
	if (LaunchWithServer)
	{
#if _WIN32
		ret = system(("start bin\\"
			+ ProjectName
			+ "-Server.exe -nostartupinfo -quitondisconnect -editorPath "
			+ Application::GetEditorPath()
			+ " "
			+ Args).c_str());
#else
		int ret = system(("./bin/"
		 + ProjectName 
		 + "-Server -nostartupinfo -quitondisconnect -editorPath "
		 + Application::GetEditorPath()
		 + " "
		 + Args + " &").c_str());
#endif
	}
}

void EditorUI::SetSaveSceneOnLaunch(bool NewValue)
{
	Editor::SaveSceneOnLaunch = NewValue;
}

#ifdef ENGINE_CSHARP
void EditorUI::RebuildAssembly()
{
	Log::Print("Rebuilding C# assembly...", Log::LogColor::Green);
	PipeProcessToLog("cd Scripts && dotnet build", false);
	Editor::CanHotreload = true;
}
#endif

void EditorUI::SetLaunchCurrentScene(bool NewLaunch)
{
	Editor::LaunchCurrentScene = NewLaunch;
}

void EditorUI::SaveCurrentScene()
{
	if (Scene::CurrentScene.empty())
	{
		Log::Print("Saving scene \"Untitled\"", Vector3(0.3f, 0.4f, 1));
		Scene::SaveSceneAs("Content/Untitled");
	}
	else
	{
		Log::Print("Saving scene \"" + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene) + "\"", Vector3(0.3f, 0.4f, 1));
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
		/*
		new DialogBox(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene), 0, "Scene has unsaved changes. Save?",
			{
				DialogBox::Answer("Yes", []()
				{
					SaveCurrentScene(); 
					ChangedScene = false;
					Scene::LoadNewScene(EditorSceneToOpen);
					Viewport::ViewportInstance->ClearSelectedObjects();
					Scene::Tick();
					Application::EditorInstance->UIElements[5]->UpdateLayout();
					Application::EditorInstance->UIElements[6]->UpdateLayout();
				}),

				DialogBox::Answer("No", []() 
				{
					ChangedScene = false;
					Scene::LoadNewScene(EditorSceneToOpen);
					Viewport::ViewportInstance->ClearSelectedObjects();
					Scene::Tick();
					Application::EditorInstance->UIElements[5]->UpdateLayout();
					Application::EditorInstance->UIElements[6]->UpdateLayout();
				}),

				DialogBox::Answer("Cancel", nullptr)
			});*/
		return;
	}
	ChangedScene = false; 
	Scene::LoadNewScene(NewScene);
	Scene::Tick();

	UpdateAllInstancesOf<ContextMenu>();
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
		std::swap(Application::EditorInstance->UIColors, Editor::NewUIColors);
		Editor::LightMode = NewLightMode;
		Log::Print("Toggled light mode", Log::LogColor::Yellow);
	}
	UIBox::ForceUpdateUI();
}

void EditorUI::PipeProcessToLog(std::string Command, bool Async)
{
	using namespace Editor;

#if _WIN32
	auto process = _popen(Command.c_str(), "r");
#else
	auto process = popen(Command.c_str(), "r");
#endif

	ProcessInfo proc;

	proc.Active = true;
	proc.Command = Command;
	proc.Pipe = process;
	proc.Async = Async;
	proc.Thread = new std::thread(ReadProcessPipe, process);

	if (!Async)
	{
		proc.Thread->join();
	}
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
	Application::EditorInstance = this;
	Text = new TextRenderer(Application::GetEditorPath() + "/EditorContent/EditorFont.ttf");
	MonoText = new TextRenderer(Application::GetEditorPath() + "/EditorContent/EditorMonospace.ttf");
	Input::CursorVisible = true;
	LoadEditorTextures();

	RootPanel = new EditorPanel(-1, Vector2(2, 1.95f), "root");
	RootPanel->ChildrenAlign = EditorPanel::ChildrenType::Horizontal;

	EditorPanel* RightPanel = new EditorPanel(RootPanel, "panel");
	RightPanel->Size = 0.3f;

	(new AssetBrowser(RightPanel));
	(new ClassesBrowser(RightPanel));

	EditorPanel* CenterPanel = new EditorPanel(RootPanel, "panel");
	CenterPanel->ChildrenAlign = EditorPanel::ChildrenType::Vertical;
	CenterPanel->Size = 1.425f;
	(new LogUI(CenterPanel))->Size = 0.4f;
	(new Viewport(CenterPanel))->Size = 1.425f;
	new Toolbar(CenterPanel);
	delete new SettingsPanel(nullptr);
	EditorPanel* LeftPanel = new EditorPanel(RootPanel, "panel");
	LeftPanel->ChildrenAlign = EditorPanel::ChildrenType::Vertical;
	EditorPanel* BottomLeftPanel = new EditorPanel(LeftPanel, "panel");
	new ContextMenu(BottomLeftPanel, false);
	new ContextMenu(BottomLeftPanel, true);
	BottomLeftPanel->Size = 0.8f;
	new ObjectList(LeftPanel);

	new StatusBar();

	RootPanel->OnPanelResized();

	Cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	Cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	Cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	Cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	Cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	Cursors[6] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	Cursors[7] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

	Console::RegisterCommand(Console::Command("build", []() {new std::thread(Build::TryBuildProject, "Build/"); }, {}));
	Console::RegisterCommand(Console::Command("save", EditorUI::SaveCurrentScene, {}));
#ifdef ENGINE_CSHARP
	Console::RegisterCommand(Console::Command("reload", EditorUI::RebuildAssembly, {}));
	Console::RegisterCommand(Console::Command("run", EditorUI::LaunchInEditor, {}));
	Console::RegisterCommand(Console::Command("toggle_light", []() { EditorUI::SetUseLightMode(!EditorUI::GetUseLightMode()); }, {}));
	Console::RegisterCommand(Console::Command("classtree", []()
		{
			//auto Items = dynamic_cast<ItemBrowser*>(Application::EditorInstance->UIElements[3])->GetEditorUIClasses();
		}, {}));

#endif
}

void(*QuitFunction)();

void EditorUI::OnLeave(void(*ReturnF)())
{
	QuitFunction = ReturnF;
	if (ChangedScene)
	{
		/*new DialogBox("Scene", 0,
			"Save changes to scene before quitting?",
			{
				DialogBox::Answer("Yes", []() {
				if (Viewport::ViewportInstance->CurrentTab)
				{
					Viewport::ViewportInstance->CurrentTab->Save();
				}
				SaveCurrentScene();
				QuitFunction(); }),
				DialogBox::Answer("No", ReturnF),
				DialogBox::Answer("Cancel", nullptr)
			});
		*/
	}
	else
	{
		/*if (Viewport::ViewportInstance->CurrentTab)
		{
			Viewport::ViewportInstance->CurrentTab->Save();
		}*/
		ReturnF();
	}
}

void EditorUI::Tick()
{
	for (auto& i : Editor::ProcessPipes)
	{
		if (!i.second.Active)
		{
			if (i.second.Async)
			{
				i.second.Thread->join();
			}
			delete i.second.Thread;
			Editor::ProcessPipes.erase(i.first);
			break;
		}
	}
	EditorPanel::TickPanels();
#ifdef ENGINE_CSHARP
	if (Editor::CanHotreload == true)
	{
		Log::Print("Finished building assembly. Hotreloading .dll file.", Log::LogColor::Yellow);
		CSharp::ReloadCSharpAssembly();
		//ItemBrowser* Browser = dynamic_cast<ItemBrowser*>(UIElements[3]);
		//Browser->CPPClasses = Browser->GetEditorUIClasses();
		//Browser->UpdateLayout();

		Editor::CanHotreload = false;
	}
#endif

	if (BakedLighting::FinishedBaking)
	{
		BakedLighting::FinishedBaking = false;
		Editor::IsBakingScene = false;
		Assets::ScanForAssets();
		BakedLighting::LoadBakeFile(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene));
	}

	if (dynamic_cast<UIButton*>(UI::HoveredBox))
	{
		CurrentCursor = CursorType::Grab;
	}
	if (dynamic_cast<UITextField*>(UI::HoveredBox))
	{
		CurrentCursor = CursorType::TextHover;
	}
	if (UIScrollBox::IsDraggingScrollBox)
	{
		CurrentCursor = CursorType::Default;
	}
	if (CurrentCursor < CursorType::End && (int)CurrentCursor >= 0)
	{
		SDL_SetCursor(Cursors[(int)CurrentCursor]);
	}
	CurrentCursor = CursorType::Default;

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

	if (Input::IsKeyDown(Input::Key::LCTRL) && Input::IsKeyDown(Input::Key::s))
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
	Dropdown = new UIBox(UIBox::Orientation::Vertical, Position);
	auto Background = new UIBackground(UIBox::Orientation::Vertical, 0, Vector3::Lerp(EditorUI::UIColors[0], EditorUI::UIColors[2], 0.5f), 0);
	Dropdown->AddChild(Background
		->SetPadding(0));

	for (size_t i = 0; i < Menu.size(); i++)
	{
		float PaddingSize = 2.0f / Graphics::WindowResolution.Y;
		bool Upper = i == 0;
		bool Lower = i == Menu.size() - 1 || Menu[i].Seperator;

		float Horizontal = 2.0f / Graphics::WindowResolution.X;

		Background->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 1.5f, this, (int)i))
			->SetPadding(PaddingSize * (float)Upper, PaddingSize * (float)Lower, Horizontal, Horizontal)
			->SetMinSize(Vector2(0.15f, 0))
			->AddChild((new UIText(0.45f, EditorUI::UIColors[2], Menu[i].Title, EditorUI::Text))
				->SetPadding(0.005f)));
	}

	UIBox::DrawAllUIElements();
	Dropdown->SetPosition((Dropdown->GetPosition() - Vector2(0, Dropdown->GetUsedSize().Y))
		.Clamp(Vector2(-1, -1), Vector2(1 - Dropdown->GetUsedSize().X, 1 - Dropdown->GetUsedSize().Y)));
}

void EditorUI::OnObjectSelected()
{
	UpdateAllInstancesOf<ContextMenu>();
	UpdateAllInstancesOf<ObjectList>();
}

std::string EditorUI::ToShortString(float val)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << val;
	return stream.str();
}


void EditorUI::LoadEditorTextures()
{
	const int ImageSize = 28;
	std::string Images[ImageSize]
	{								//Texture Indices
		"CPPClass.png",				//00 -> C++ class icon
		"Wireframe.png",			//01 -> Symbol for button to toggle wireframe
		"Save.png",					//02 -> Save Button
		"Build.png",				//03 -> Package button
		"X.png",					//04 -> X Symbol
		"Folder.png",				//05 -> Folder symbol for asset browser
		"Sound.png",				//06 -> Sound symbol for asset browser
		"Scene.png",				//07 -> Scene symbol for asset browser
		"ExitFolder.png",			//08 -> Icon used to navigate back one folder
		"Material.png",				//09 -> Material symbol for asset browser
		"MaterialTemplate.png",		//10 -> Material Template symbol for asset browser
		"Model.png",				//11 -> Model symbol for asset browser
		"Reload.png",				//12 -> Reload symbol
		"ExpandedArrow.png",		//13 -> Expanded arrow
		"CollapsedArrow.png",		//14 -> Collapsed arrow
		"Placeholder.png",			//15 -> Placeholder
		"Checkbox.png",				//16 -> Checked checkbox
		"Cubemap.png",				//17 -> Cubemap icon
		"Texture.png",				//18 -> Texture icon
		"Particle.png",				//19 -> Particle icon
		"Settings.png",				//20 -> Settings icon
		"Play.png",					//21 -> Play icon
		"CSharpClass.png",			//22 -> CSharp class icon
		"WindowX.png",				//23 -> Window X icon
		"WindowResize.png",			//24 -> Window Resize icon
		"WindowResize2.png",		//25 -> Window Fulscreen Resize icon
		"WindowMin.png",			//26 -> Window Minimize icon
		"Lighting.png"				//27 -> Lightmap icon	
	};

	for (int i = 0; i < Textures.size(); i++)
	{
		Texture::UnloadTexture(Textures[i]);
	}
	for (int i = 0; i < ImageSize; i++)
	{
		Textures.push_back(Texture::LoadTexture(Application::GetEditorPath() + "/EditorContent/Images/" + Images[i]));
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
			ObjectList[ObjectList.size() - 1].ListIndex = (int)ObjectCategories.size() - 1;
			SceneList = &ObjectList[ObjectList.size() - 1];
		}

		// Seperate the Object's category into multiple strings
		std::string CurrentPath = Objects::GetCategoryFromID(o->GetObjectDescription().ID);
		if (o->GetObjectDescription().ID == CSharpObject::GetID() && static_cast<CSharpObject*>(o)->CS_Obj.ID != 0)
		{
			auto Classes = CSharp::GetAllClasses();
			for (auto& i : Classes)
			{
				size_t LastSlash = i.find_last_of("/");
				std::string Path;
				std::string Name = i;
				if (LastSlash != std::string::npos)
				{
					Path = i.substr(0, LastSlash);
					Name = i.substr(LastSlash);
				}
				if (Name == static_cast<CSharpObject*>(o)->CSharpClass)
				{
					CurrentPath = Path;
				}
			}
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
				CurrentList->Children.insert(CurrentList->Children.begin() + it,
					ObjectListItem(elem, {}, false, CollapsedItems.contains("OBJ_CAT_" + elem)));
				CurrentList->Children[it].ListIndex = (int)ObjectCategories.size() - 1;
				NewList = &CurrentList->Children[it];
			}
			CurrentList = NewList;
		}
		if (CurrentList && !CurrentList->IsCollapsed)
		{
			CurrentList->Children.push_back(ObjectListItem(o, (int)ListIndex));
		}
		else if (CurrentList && o->IsSelected)
		{
			CurrentList->IsSelected = true;
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

void EditorUI::BakeScene()
{
	Editor::IsBakingScene = true;
	BakedLighting::BakeCurrentSceneToFile();
}

void EditorUI::OnResized()
{
	RootPanel->OnPanelResized();
}

#endif