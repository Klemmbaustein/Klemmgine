#pragma once
#include "Objects/WorldObject.h"
#include <fstream>
class Camera;

/**
* @file
* @brief 
* Functions related to loading/saving scene files (*.jscn)
*/

/**
* @brief 
* Namespace containing functions related to scenes.
* 
* All behaviour related to subscenes is deprecated.
* 
* @todo Fix/reimplement subscenes
*/
namespace Scene
{
	extern Camera* DefaultCamera;

	/**
	* @brief
	* Path to the currently loaded Scene, without extension.
	* 
	* Contains the path to the currently loaded scene, without the extension.
	* So for the scene `Content/Scenes/Scene.jscn`, this will be set to `Content/Scenes/Scene`
	*/
	extern std::string CurrentScene;

	/**
	* Saves the current scene to the path.
	* 
	* @param FilePath
	* The path to the location the file should be saved to.
	* 
	* @param Subscene
	* Parameter controlling if the scene to save is a Subscene
	*/
	void SaveSceneAs(std::string FilePath, bool Subscene = false);
	
	/**
	* @deprecated
	* @brief Loads a subscene (.subscn file).
	* Loading a subscene will not unload the currently loaded scene, only create new objects from the subscene.
	* 
	* @param File
	* The **File name** of the scene. If the file's path is `Content/Scene.subscn`, the name is only `Scene`
	*/
	void LoadSubScene(std::string File);

	void Tick();

	/**
	* Loads a scene (.jscn file). This will unload the previously loaded scene.
	* 
	* @param File
	* The **File name** of the scene. If the file's path is `Content/Scene.subscn`, the name is only `Scene`
	*/
	void LoadNewScene(std::string File);
}