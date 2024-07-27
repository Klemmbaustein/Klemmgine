#ifdef EDITOR
#include "EditorUI.h"
#include "Engine/Utility/FileUtility.h"
#include <Objects/CSharpObject.h>
#include <filesystem>
#include "Math/Math.h"
#include "Math/Collision/Collision.h"
#include "Engine/Subsystem/Scene.h"
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
#include <Engine/Subsystem/BackgroundTask.h>
#include <Engine/Input.h>
#include <UI/EditorUI/Popups/DialogBox.h>
#include <Engine/Subsystem/Console.h>
#include <Engine/Subsystem/Console.h>
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Engine/File/Assets.h>
#include <UI/UIDropdown.h>
#include <SDL.h>
#include <Rendering/Texture/Texture.h>
#include <Engine/Application.h>
#include <UI/EditorUI/SettingsPanel.h>
#include <Networking/Networking.h>
#include <UI/EditorUI/SerializePanel.h>
#include <Engine/Utility/StringUtility.h>

int EditorUI::NumLaunchClients = 1;
TextRenderer* EditorUI::Text = nullptr;
TextRenderer* EditorUI::MonoText = nullptr;
EditorPanel* EditorUI::RootPanel = nullptr;
std::vector<unsigned int> EditorUI::Textures;
std::vector<SceneObject*> EditorUI::SelectedObjects;
EditorUI::CursorType EditorUI::CurrentCursor = CursorType::Default;

// TODO: Move run-in-editor and C# hotreload stuff into subsystems. (for prettier logging and cleaner code)

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
	bool LaunchCurrentScene = true;
	bool SaveSceneOnLaunch = false;
	bool ReloadingCSharp = false;
	DialogBox* CSharpReloadBox = nullptr;

	bool Rebuilding = false;
	DialogBox* RebuildingBox = nullptr;

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
		std::string LogPrefix;
		std::thread* Thread = nullptr;
		std::atomic<bool> Active = false;
		bool Async = false;
	};

	static void ReadProcessPipe(FILE* p, ProcessInfo* Info)
	{
		std::string CurrentMessage;
		while (!feof(p))
		{
			char NewChar = (char)fgetc(p);

			if (NewChar == '\n')
			{
				Log::Print(Info->LogPrefix + CurrentMessage);
				CurrentMessage.clear();
			}
			else
			{
				CurrentMessage.append({ NewChar });
			}
		}
		fclose(p);

		Info->Active = false;
	}
}

namespace UI
{
	extern std::vector<UIBox*> UIElements;
}

bool EditorUI::ChangedScene = false;
bool EditorUI::IsBakingScene = false;
std::string EditorUI::LaunchInEditorArgs;
bool EditorUI::LaunchWithServer = false;
static std::string LaunchCommandLine;
void EditorUI::LaunchInEditor()
{
	std::string ProjectName = Build::GetProjectBuildName();
	std::string ExecutablePath = "", ServerExecutablePath = "";
	try
	{
#if !ENGINE_NO_SOURCE
		// No Solution -> no build name -> probably using CMake on windows.
		if (Build::CMake::IsUsingCMake())
		{
			std::string MsBuildConfig = Build::CMake::GetMSBuildConfig();

#if _WIN32
			std::string ExecExtension = ".exe";
#else
			std::string ExecExtension = "";
#endif

			std::string CMakeConfigName = "x64-Debug";
			ExecutablePath = Build::CMake::GetBuildRootPath(CMakeConfigName) + "\\";
#if _WIN32
			if (std::filesystem::exists(ExecutablePath + MsBuildConfig))
			{
				ExecutablePath.append(MsBuildConfig + "\\");
			}
#endif

			ExecutablePath.append(ProjectName + ExecExtension);

			if (!std::filesystem::exists(ExecutablePath)
				|| std::filesystem::last_write_time(ExecutablePath) < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" }))
			{
				Editor::Rebuilding = true;
				Log::Print("Detected uncompiled changes to C++ code. Rebuilding...", Log::LogColor::Yellow);
				Build::CMake::BuildWithConfig(CMakeConfigName);
			}

			CMakeConfigName = "x64-Server";
			ServerExecutablePath = Build::CMake::GetBuildRootPath(CMakeConfigName) + "\\";

#if _WIN32
			if (std::filesystem::exists(ServerExecutablePath + MsBuildConfig))
			{
				ServerExecutablePath.append(MsBuildConfig + "\\");
			}
#endif

			ServerExecutablePath.append(ProjectName + ExecExtension);
			if (LaunchWithServer
				&& (!std::filesystem::exists(ServerExecutablePath)
					|| std::filesystem::last_write_time(ServerExecutablePath) < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" })))
			{
				Editor::Rebuilding = true;
				Log::Print("Detected uncompiled changes to C++ code. Rebuilding...", Log::LogColor::Yellow);
				Build::CMake::BuildWithConfig(CMakeConfigName, "-DSERVER=ON");
			}
		}
#if _WIN32
		else
		{
			ExecutablePath = "bin\\" + ProjectName + "-Debug.exe";
			if (!std::filesystem::exists(ExecutablePath)
				|| std::filesystem::last_write_time(ExecutablePath) < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" }))
			{
				Editor::Rebuilding = true;
				Log::Print("Detected uncompiled changes to C++ code. Rebuilding...", Log::LogColor::Yellow);
				if (Build::BuildCurrentSolution("Debug"))
				{
					Log::Print("Build for configuration 'Debug' failed. Cannot launch project.", Log::LogColor::Red);
					Editor::Rebuilding = false;
					return;
				}
			}

			ServerExecutablePath = "bin\\" + ProjectName + "-Server.exe";
			if (LaunchWithServer 
				&& (!std::filesystem::exists(ServerExecutablePath)
					|| std::filesystem::last_write_time(ServerExecutablePath) < FileUtil::GetLastWriteTimeOfFolder("Code", { "x64" })))
			{
				Editor::Rebuilding = true;
				if (Build::BuildCurrentSolution("Server"))
				{
					Log::Print("Build for configuration 'Debug' failed. Cannot launch project.", Log::LogColor::Red);
					Editor::Rebuilding = false;
					return;
				}
			}
		}
#endif // WIN32
		Editor::Rebuilding = false;
#endif // !ENGINE_NO_SOURCE

#if ENGINE_NO_SOURCE || __linux__
#if !ENGINE_NO_SOURCE
		if (!Build::CMake::IsUsingCMake())
#endif
		{
			ProjectName = "Klemmgine";
			ExecutablePath = "bin\\" + ProjectName + "-Debug";
			ServerExecutablePath = "bin\\" + ProjectName + "-Server";
		}
#if _WIN32
		ExecutablePath.append(".exe");
		ServerExecutablePath.append(".exe");
#endif
#endif // ENGINE_NO_SOURCE || __linux__

#ifdef ENGINE_CSHARP
		if ((!std::filesystem::exists("CSharp/Build/CSharpAssembly.dll")
			|| std::filesystem::last_write_time("CSharp/Build/CSharpAssembly.dll") < FileUtil::GetLastWriteTimeOfFolder("Scripts", { "obj" }))
			&& CSharpInterop::GetUseCSharp())
		{
			RebuildAssembly();
		}
#endif
	}
	catch (std::exception& e)
	{
		Log::Print("Exception thrown when trying to check for rebuild. " + std::string(e.what()));
		return;
	}

	if (Editor::SaveSceneOnLaunch)
	{
		Application::EditorInstance->ShouldSave = true;
	}

	std::string Args = LaunchInEditorArgs;
	if (Editor::LaunchCurrentScene)
	{
		Args.append(" -scene " + FileUtil::GetFileNameFromPath(Scene::CurrentScene));
	}

#if __linux__
	StrUtil::ReplaceChar(ExecutablePath, '\\', "/");
#endif

	std::string CommandLine = ExecutablePath + " -nostartupinfo -nocolor -editorPath " + Application::GetEditorPath() + " " + Args;

	if (LaunchWithServer)
	{
		CommandLine.append(" -connect localhost ");
	}

	Log::Print("[Debug]: Starting process: " + CommandLine, Log::LogColor::Blue);
	int ret = 0;
	std::string Command;
#if _WIN32
	for (int i = 0; i < NumLaunchClients; i++)
	{
		Command.append("start /b " + CommandLine);
		if (i < NumLaunchClients - 1)
		{
			Command.append(" && ");
		}
	}

	// If only one new process is created, write the output of that process to the log.
	// With multiple processes this gets very confusing.
	if (NumLaunchClients == 1)
	{
		new BackgroundTask([Command]() { PipeProcessToLog(Command, "[Debug]: "); });
	}
	else
	{
		new BackgroundTask([Command]() { int ret = system(Command.c_str()); });
	}
#else
	for (int i = 0; i < NumLaunchClients; i++)
	{
		LaunchCommandLine = CommandLine;
		if (NumLaunchClients == 1)
		{
			PipeProcessToLog(LaunchCommandLine, "[Debug]: ");
}
		else
		{
			new BackgroundTask([]() { system((LaunchCommandLine).c_str()); });
		}
	}
#endif
	if (LaunchWithServer)
	{
#if _WIN32
		ret = system(("start " + ServerExecutablePath + " -nostartupinfo -quitondisconnect -editorPath "
			+ Application::GetEditorPath()
			+ " "
			+ Args).c_str());
#else
		ret = system(("./bin/"
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
	Editor::ReloadingCSharp = true;
	PipeProcessToLog("cd Scripts && dotnet build", "[C#]: [Build]: ");
	Editor::ReloadingCSharp = false;
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
		Application::EditorInstance->Print("Saving scene \"Untitled\"");
		Scene::SaveSceneAs("Content/Untitled");
		Scene::CurrentScene = "Content/Untitled";
	}
	else
	{
		Application::EditorInstance->Print("Saving scene \"" + FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene) + "\"");
		Scene::SaveSceneAs(Scene::CurrentScene);
	}
	ChangedScene = false;
	AssetBrowser::UpdateAll();
	UpdateAllInstancesOf<ObjectList>();
}

void EditorUI::OpenScene(std::string NewScene)
{
	if (ChangedScene)
	{
		new DialogBox(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene),
			0, 
			"The current scene has unsaved changes. Save these changes before loading a new scene?",
			{
				EditorPopup::PopupOption("Yes", [NewScene]()
				{
					SaveCurrentScene(); 
					ChangedScene = false;
					Scene::LoadNewScene(NewScene, true);
					Viewport::ViewportInstance->ClearSelectedObjects();
					UpdateAllInstancesOf<ContextMenu>();
				}),

				EditorPopup::PopupOption("No", [NewScene]()
				{
					ChangedScene = false;
					Scene::LoadNewScene(NewScene, true);
					Viewport::ViewportInstance->ClearSelectedObjects();
					UpdateAllInstancesOf<ContextMenu>();
				}),

				EditorPopup::PopupOption("Cancel", nullptr)
			});
		return;
	}
	ChangedScene = false; 
	Scene::LoadNewScene(NewScene, true);

	EditorUI::SelectedObjects.clear();

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
				static_cast<UIText*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UIText*>(i)->GetColor()));
			}
			if (dynamic_cast<UIButton*>(i))
			{
				static_cast<UIButton*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UIButton*>(i)->GetColor()));
			}
			if (dynamic_cast<UIBackground*>(i))
			{
				static_cast<UIBackground*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UIBackground*>(i)->GetColor()));
			}
			if (dynamic_cast<UITextField*>(i))
			{
				static_cast<UITextField*>(i)->SetColor(Editor::ReplaceWithNewUIColor(static_cast<UITextField*>(i)->GetTextColor()));
				static_cast<UITextField*>(i)->SetTextColor(Editor::ReplaceWithNewUIColor(static_cast<UITextField*>(i)->GetTextColor()));
			}
		}
		std::swap(Application::EditorInstance->UIColors, Editor::NewUIColors);
		Editor::LightMode = NewLightMode;
		Application::EditorInstance->Print("Toggled light mode", Subsystem::ErrorLevel::Warn);
	}
	UIBox::ForceUpdateUI();
}

void EditorUI::LoadDefaultLayout()
{
	if (RootPanel)
	{
		delete RootPanel;
	}

	RootPanel = new EditorPanel(-1, Vector2(2, 1.95f), "root", "root");
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
	EditorPanel* LeftPanel = new EditorPanel(RootPanel, "panel");
	LeftPanel->ChildrenAlign = EditorPanel::ChildrenType::Vertical;
	EditorPanel* BottomLeftPanel = new EditorPanel(LeftPanel, "panel");
	new ContextMenu(BottomLeftPanel, false);
	new ContextMenu(BottomLeftPanel, true);
	BottomLeftPanel->Size = 0.8f;
	new ObjectList(LeftPanel);

	RootPanel->OnPanelResized();
}

void EditorUI::LoadPanelLayout(EditorPanel* From)
{
	if (RootPanel)
	{
		delete RootPanel;
		RootPanel = nullptr;
	}
	RootPanel = From;
}

void EditorUI::PipeProcessToLog(std::string Command, std::string Prefix)
{
	using namespace Editor;

#if _WIN32
	FILE* process = _popen(Command.c_str(), "r");
#else
	auto process = popen(Command.c_str(), "r");
#endif

	ProcessInfo proc;

	proc.LogPrefix = Prefix;
	proc.Active = true;
	proc.Command = Command;
	proc.Pipe = process;

	ReadProcessPipe(process, &proc);
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
	Name = "UISystem";
	Text = new TextRenderer(Application::GetEditorPath() + "/EditorContent/EditorFont.ttf");
	MonoText = new TextRenderer(Application::GetEditorPath() + "/EditorContent/EditorMonospace.ttf");
	Input::CursorVisible = true;
	LoadEditorTextures();

	if (std::filesystem::exists(Editor::SerializePanel::GetLayoutPrefFilePath() + ".pref"))
	{
		SaveData s = SaveData(Editor::SerializePanel::GetLayoutPrefFilePath(), "pref", false, false);
		EditorUI::LoadPanelLayout(Editor::SerializePanel::DeSerializeLayout(s.GetField("root")));
	}
	else
	{
		LoadDefaultLayout();
	}
	new StatusBar();
	delete new SettingsPanel(nullptr);

	Cursors[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	Cursors[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	Cursors[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	Cursors[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	Cursors[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	Cursors[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	Cursors[6] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	Cursors[7] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

	Console::ConsoleSystem->RegisterCommand(Console::Command("build", []() {new std::thread(Build::TryBuildProject, "GameBuild/"); }, {}));
	Console::ConsoleSystem->RegisterCommand(Console::Command("save", []() {
		std::string Old = Scene::CurrentScene;
		if (!Console::ConsoleSystem->CommandArgs().empty())
		{
			Scene::CurrentScene = "Content/" + Console::ConsoleSystem->CommandArgs()[0];
		}

		EditorUI::SaveCurrentScene();
		Scene::CurrentScene = Old;
		}, { Console::Command::Argument("scene_file_path", NativeType::String, true)}));
	Console::ConsoleSystem->RegisterCommand(Console::Command("dump_editor_layout", [this]() 
		{
			Print(Editor::SerializePanel::SerializeLayout(RootPanel).Serialize(0));
		}, {}));

	Console::ConsoleSystem->RegisterCommand(Console::Command("toggle_light", []() { EditorUI::SetUseLightMode(!EditorUI::GetUseLightMode()); }, {}));
#ifdef ENGINE_CSHARP
	Console::ConsoleSystem->RegisterCommand(Console::Command("reload", EditorUI::RebuildAssembly, {}));
	Console::ConsoleSystem->RegisterCommand(Console::Command("run", EditorUI::LaunchInEditor, {}));

#endif
}

EditorUI::~EditorUI()
{
}

void EditorUI::OnLeave()
{
	if (ChangedScene)
	{
		new DialogBox("Scene", 0,
			"The current scene has unsaved changes. Save changes before exiting?",
			{
				EditorPopup::PopupOption("Yes", []() { SaveCurrentScene(); Application::Quit(); }),
				EditorPopup::PopupOption("No", &Application::Quit),
				EditorPopup::PopupOption("Cancel", nullptr)
			});
	}
	else
	{
		Application::Quit();
	}
}

void EditorUI::Update()
{
	EditorPanel::TickPanels();
#ifdef ENGINE_CSHARP
	if (Editor::CanHotreload == true)
	{
		Print("Finished building assembly. HotReloading .dll file.", Subsystem::ErrorLevel::Info);
		CSharpInterop::CSharpSystem->ReloadCSharpAssembly();
		for (UICanvas* c : Graphics::UIToRender)
		{
			auto Browser = dynamic_cast<ClassesBrowser*>(c);
			if (Browser)
			{
				Browser->UpdateClasses();
			}
		}

		UpdateAllInstancesOf<ClassesBrowser>();

		Editor::CanHotreload = false;
	}
#endif

	if (BakedLighting::FinishedBaking)
	{
		BakedLighting::FinishedBaking = false;
		EditorUI::IsBakingScene = false;
		Assets::ScanForAssets();
		BakedLighting::LoadBakeFile(FileUtil::GetFileNameWithoutExtensionFromPath(Scene::CurrentScene));
	}

	if (Editor::ReloadingCSharp && !Editor::CSharpReloadBox)
	{
		Editor::CSharpReloadBox = new DialogBox("C#", 0, "Rebuilding C# assembly... Check log for details.", {});
	}
	else if (!Editor::ReloadingCSharp && Editor::CSharpReloadBox)
	{
		delete Editor::CSharpReloadBox;
		Editor::CSharpReloadBox = nullptr;
	}

	if (Editor::Rebuilding && !Editor::RebuildingBox)
	{
		Editor::RebuildingBox = new DialogBox("Build", 0, "Compiling game...", {});
	}
	else if (!Editor::Rebuilding && Editor::RebuildingBox)
	{
		delete Editor::RebuildingBox;
		Editor::RebuildingBox = nullptr;
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

	if (DraggedItem)
	{
		DraggedItem->SetPosition(Input::MouseLocation - DraggedItem->GetUsedSize() / 2);
		if (!Input::IsLMBDown)
		{
			delete DraggedItem;
			DraggedItem = nullptr;
		}
	}

	if ((Input::IsKeyDown(Input::Key::LCTRL) && Input::IsKeyDown(Input::Key::s) && !Input::IsRMBDown) || ShouldSave)
	{
		if (ChangedScene)
		{
			SaveCurrentScene();
			ShouldSave = false;
		}
		Editor::IsSavingScene = true;
	}
	else
	{
		Editor::IsSavingScene = false;
	}
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
		"Cubemap.png",				//17 -> CubeMap icon
		"Texture.png",				//18 -> Texture icon
		"Particle.png",				//19 -> Particle icon
		"Settings.png",				//20 -> Settings icon
		"Play.png",					//21 -> Play icon
		"CSharpClass.png",			//22 -> CSharp class icon
		"WindowX.png",				//23 -> Window X icon
		"WindowResize.png",			//24 -> Window Resize icon
		"WindowResize2.png",		//25 -> Window FullScreen Resize icon
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

void EditorUI::BakeScene()
{
	EditorUI::IsBakingScene = true;
	BakedLighting::BakeCurrentSceneToFile();
}

void EditorUI::OnResized()
{
	RootPanel->OnPanelResized();
}

#endif