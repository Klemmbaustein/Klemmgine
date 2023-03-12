#pragma once
#include "Objects/WorldObject.h"
#include <fstream>

class Camera;

namespace Scene
{
	extern Camera* DefaultCamera;

	struct Object
	{
		Transform ObjectTransform;
	};

	extern std::string CurrentScene;

	void SaveSceneAs(std::string FilePath, bool Subscene = false);
	void LoadSubScene(std::string File);

	void Tick();
	void LoadNewScene(std::string File);
}