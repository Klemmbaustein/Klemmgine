#ifdef EDITOR
#pragma once
#include "UI/Default/UICanvas.h"
#include "UI/Default/TextRenderer.h"
#include <UI/UIBackground.h>
#include "UI/UIButton.h"
#include <UI/UITextField.h>
#include <UI/UIText.h>
#include <UI/EditorUI/EditorPanel.h>
#include <Engine/TypeEnun.h>
#include <Objects/WorldObject.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Build/Build.h>

class EditorUI;
class UIVectorField;
class UITextField;
struct SDL_Cursor;
extern bool ChangedScene;

/**
* @defgroup Editor
* 
* @brief
* Functions/classes related to the Editor.
*/

/**
* @brief
* Class controlling the Editor and Editor UI.
* 
* This class contains functions for managing the Editor.
* 
* @ingroup Editor
*/
class EditorUI : public UICanvas
{
public:
#if ENGINE_CSHARP
	/**
	* @brief
	* Rebuilds the project's CSharpAssembly and reloads it.
	*/
	static void RebuildAssembly();
#endif
	
	/**
	* @brief 
	* Launches a debug process of the current project.
	* 
	* This function will detect changes to the C++ source code in {ProjectPath}/Code/ and rebuild the Debug configuration if necessary.
	* It will also detect changes to the C# code in {ProjectPath}/Scripts and rebuild it as well.
	*/
	static void LaunchInEditor();
	static void SetSaveSceneOnLaunch(bool NewValue);
	static std::string LaunchInEditorArgs;
	static void SetLaunchCurrentScene(bool NewLaunch);

	/**
	* @brief
	* Saves the scene currently opened in the editor.
	* 
	* If the loaded scene name is empty, the scenes name will be "Untitled"
	*/
	static void SaveCurrentScene();

	/**
	* @brief 
	* Loads the given scene. Creates a "Save changes" dialog if necessary.
	* 
	* @param NewScene
	* The scene to load.
	*/
	static void OpenScene(std::string NewScene);
	static bool GetUseLightMode();
	static void SetUseLightMode(bool NewLightMode);
	static int NumLaunchClients;
	static bool LaunchWithServer;

	/**
	* @brief
	* Calls the OnResized() function on all EditorPanel instances of the type T.
	* 
	* @tparam T
	* The type of EditorPanel that should be updated.
	*/
	template<typename T>
	static void UpdateAllInstancesOf()
	{
		for (UICanvas* c : Graphics::UIToRender)
		{
			if (dynamic_cast<T*>(c))
			{
				static_cast<T*>(c)->OnResized();
			}
		}
	}

	/**
	* @brief
	* Creates a proccess from the given command, writes the output to the log.
	* 
	* @param Command
	* The Command to execute
	* 
	* @param Async
	* If true, the function will not block the current thread until the command has finished. If false, it will.
	*/
	static void PipeProcessToLog(std::string Command, bool Async);

	/**
	* @brief
	* Creates a file with the path 'Path' and the name 'Name' with the extension 'Ext'. 
	* 
	* If the file already exists, a number will be appended to the name.
	* 
	* For example, when creating the file "file" with the extension "txt" in the directory "Test/" that contains the files:
	*	file.txt
	*	file_0.txt
	* 
	* the resulting file will be called "file_1.txt"
	* 
	* @param Path
	* The path in which the file should be created.
	* 
	* @param Name
	* The name of the file.
	*
	* @param Ext
	* the extension of the file, without a dot.
	*/
	static void CreateFile(std::string Path, std::string Name, std::string Ext);

	EditorUI();

	/**
	* @brief
	* Function that creates a "Save changes" popup if necessary, then quits the application.
	* 
	* If you want to quit the application, use Application::Quit() instead.
	* 
	* @param ReturnF
	* The function that should be called to quit the applicatin.
	*/
	void OnLeave(void(*ReturnF)());
	virtual void Tick() override;

	void LoadEditorTextures();

	UIBox* DraggedItem = nullptr;
	UIBox* Dropdown = nullptr;

	static inline constexpr uint32_t NumUIColors = 4;

	/**
	* @brief
	* Array containing colors for the Editor UI to use.
	* 
	* Colors are:
	* - Default background
	* - Dark background
	* - Highlight color
	* 
	* These colors will be changed for light mode.
	*/
	static Vector3 UIColors[NumUIColors];

	/**
	* @brief
	* The default font for the editor.
	* 
	* Uses the font {Application::GetEditorPath()}/EditorContent/EditorFont.ttf
	*/
	static TextRenderer* Text;

	/**
	* @brief
	* The monospace font for the editor.
	* 
	* Uses the font {Application::GetEditorPath()}/EditorContent/EditorMonospace.ttf
	*/
	static TextRenderer* MonoText;

	enum class CursorType
	{
		Default = 0,
		Grab = 1,
		Cross = 2,
		Loading = 3,
		Resize_WE = 4,
		Resize_NS = 5,
		Resize_All = 6,
		TextHover = 7,
		End
	};

	struct DropdownItem
	{
		std::string Title;
		void (*OnPressed)() = nullptr;
		bool Seperator = false;
	};

	void ShowDropdownMenu(std::vector<DropdownItem> Menu, Vector2 Position);

	static EditorPanel* RootPanel;

	static CursorType CurrentCursor;
	static std::vector<unsigned int> Textures;

protected:
	SDL_Cursor* Cursors[(int)CursorType::End];
	std::vector<DropdownItem> CurrentDropdown;

public:
	static std::vector<WorldObject*> SelectedObjects;
	static void OnObjectSelected();
	std::set<std::string> CollapsedItems;
	std::vector<std::string> ObjectCategories;


	static std::string ToShortString(float val);

	struct ObjectListItem
	{
		ObjectListItem(std::string CategoryName, std::vector<ObjectListItem> Children, bool IsScene, bool IsCollapsed)
		{
			this->Name = CategoryName;
			this->Children = Children;
			this->IsScene = IsScene;
			this->IsCollapsed = IsCollapsed;
		}
		ObjectListItem(WorldObject* Object, int ListIndex)
		{
			this->Object = Object;
			this->ListIndex = ListIndex;
			if (Object)
			{
				Name = Object->GetName(); 
				IsSelected = Object->IsSelected;
			}
		}

		bool IsSelected = false;
		bool IsCollapsed = false;
		std::vector<ObjectListItem> Children;
		std::string Name;
		WorldObject* Object = nullptr;
		int ListIndex = -1;
		bool IsScene = false;
	};
	std::vector<ObjectListItem> GetObjectList();
	void OnButtonClicked(int Index);

	void BakeScene();

	void OnResized();

	static bool IsBakingScene;
	static inline std::map<std::string, Vector3> ItemColors
	{
		std::pair("dir", Vector3(0.8f, 0.5f, 0)),
		std::pair("jsmat", Vector3(0, 0.7f, 0)),
		std::pair("jsm", Vector3(0, 0, 0.75f)),
		std::pair("jscn", Vector3(1.0f, 0.4f, 0.4f)),
		std::pair("png", Vector3(0.3f, 0, 1)),
		std::pair("cbm", Vector3(0.7f, 0.1f, 0.4f)),
		std::pair("jspart", Vector3(0.7f, 0.4f, 0.4f)),
		std::pair("wav", Vector3(0.7f, 0, 0.4f)),
		std::pair("cpp", Vector3(0.5f)),
		std::pair("setting", Vector3(1)),
		std::pair("bkdat", Vector3(1.0f, 0.6f, 0.2f)),
		std::pair("cs", Vector3(0.603921569f, 0.28627451f, 0.576470588f))
	};
	static inline std::map<std::string, unsigned int> ItemTextures
	{
		std::pair("dir", 5),
		std::pair("jsmat", 9),
		std::pair("jsm", 11),
		std::pair("jscn", 7),
		std::pair("png", 18),
		std::pair("cbm", 17),
		std::pair("wav", 6),
		std::pair("cpp", 0),
		std::pair("cs", 22),
		std::pair("jspart", 19),
		std::pair("bkdat", 27)
	};

	static inline std::set<std::string> ModelFileExtensions =
	{
		"obj",
		"fbx",
		"gltf",
		"glb",
	};

protected:
	friend WorldObject;
};
#endif