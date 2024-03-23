#pragma once
#include "Objects/WorldObject.h"
#include <fstream>
#include "Subsystem.h"
class Camera;

/**
* @file
* @brief 
* Functions related to loading/saving scene files (*.jscn)
*/

/**
* @brief 
* Subsystem managing scenes.
* 
* All behaviour related to subscenes is deprecated.
* 
* @todo Fix/reimplement subscenes
*/
class Scene : public Subsystem
{
public:
	Scene();

	static Camera* DefaultCamera;

	/**
	* @brief
	* Path to the currently loaded Scene, without extension.
	*
	* Contains the path to the currently loaded scene, without the extension.
	* So for the scene `Content/Scenes/Scene.jscn`, this will be set to `Content/Scenes/Scene`
	*/
	static std::string CurrentScene;

	/**
	* Saves the current scene to the path.
	*
	* @param FilePath
	* The path to the location the file should be saved to.
	*
	* @param Subscene
	* Parameter controlling if the scene to save is a Subscene
	*/
	static void SaveSceneAs(std::string FilePath, bool Subscene = false);

	/**
	* @deprecated
	* @brief Loads a subscene (.subscn file).
	* Loading a subscene will not unload the currently loaded scene, only create new objects from the subscene.
	*
	* @param File
	* The **File name** of the scene. If the file's path is `Content/Scene.subscn`, the name is only `Scene`
	*/
	static void LoadSubScene(std::string File);

	void Update() override;

	/**
	* Loads a scene (.jscn file). This will unload the previously loaded scene.
	*
	* @param File
	* The **File name** of the scene. If the file's path is `Content/Scene.subscn`, the name is only `Scene`
	* 
	* @param Instant
	* Should the scene be instantly loaded. If false, the scene will be loaded when the scene subsystem is updated.
	* Should never be true if executing code from an object, since the executing object will be unloaded.
	*/
	static void LoadNewScene(std::string File, bool Instant = false);

private:
	static bool ShouldLoadNewScene;
	static std::string NewLoadedScene;
	static void LoadSceneInternally(std::string FilePath);
};