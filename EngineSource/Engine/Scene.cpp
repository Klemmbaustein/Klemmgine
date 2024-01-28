#include "Scene.h"
#include "Utility/FileUtility.h"
#include <filesystem>
#include <sstream>
#include <Engine/Log.h>
#include <Sound/Sound.h>
#include <UI/Default/UICanvas.h>
#include <Engine/File/Assets.h>
#include <Rendering/Graphics.h>
#include <Engine/Stats.h>
#include <Engine/Input.h>
#include <Rendering/Utility/Framebuffer.h>
#include <UI/UIBox.h>
#include <Math/Collision/Collision.h>
#include <Rendering/Camera/CameraShake.h>
#include <Rendering/Camera/Camera.h>
#include <Rendering/Utility/BakedLighting.h>
#include <UI/Debug/DebugUI.h>
#include <Engine/EngineProperties.h>

// Old scene files do not save the fog and sun properties
#define SAVE_FOG_AND_SUN 1

namespace Scene
{
	std::string ReadBinaryStringFromFile(std::ifstream& BinFile)
	{
		int len = 0;
		BinFile.read((char*)&len, sizeof(int));
		char* temp = new char[len + 1];
		BinFile.read(temp, len);
		temp[len] = '\0';
		std::string str = temp;
		delete[] temp;
		return str;
	}

	void WriteBinaryStringToFile(std::string str, std::ofstream& BinFile)
	{
		int len = (int)str.size();
		BinFile.write((char*)&len, sizeof(int));
		BinFile.write(str.c_str(), len);
	}

	bool ShouldLoadNewScene = false;
	std::string NewLoadedScene;
	Camera* DefaultCamera = new Camera(2.5f, 1600, 900, false);

	std::string CurrentScene = "Cotent/Untitled";
	void LoadSceneInternally(std::string FilePath)
	{
		Scene::CurrentScene = FilePath;
		std::string OriginalString = FilePath;
		if (!std::filesystem::exists(FilePath))
		{
			FilePath = Assets::GetAsset(FilePath + ".jscn");
		}
		if (std::filesystem::exists(FilePath))
		{
#if !SERVER
			Graphics::MainCamera = DefaultCamera;
			if (!IsInEditor)
			{
				auto UIR = Graphics::UIToRender;
				for (UICanvas* b : UIR)
				{
					delete b;
				}
				Graphics::UIToRender.clear();
				UIBox::ClearUI();
			}
			Sound::StopAllSounds();
			if (Graphics::MainFramebuffer)
			{
				Graphics::MainFramebuffer->ReflectionCubemapName.clear();
			}
#endif
			for (int i = 0; i < Objects::AllObjects.size(); i++)
			{
				Objects::AllObjects.at(i)->IsSelected = false;
			}
			TextInput::PollForText = false;
			Debugging::EngineStatus = "Loading Scene";
			for (int i = 0; i < Objects::AllObjects.size(); i++)
			{
				if (Objects::AllObjects[i] != nullptr)
				{
					Objects::DestroyObject(Objects::AllObjects[i]);
				}
			}
			WorldObject::DestroyMarkedObjects();

			Objects::AllObjects.clear();
#if !SERVER
			if (!IsInEditor)
			{
				for (FramebufferObject* f : Graphics::AllFramebuffers)
				{
					if (f != Graphics::MainFramebuffer)
					{
						delete f;
					}
				}
				if (Graphics::MainFramebuffer)
				{
					Graphics::AllFramebuffers.clear();
					Graphics::AllFramebuffers.push_back(Graphics::MainFramebuffer);
				}
			}
			if (Graphics::MainFramebuffer)
			{
				Graphics::MainFramebuffer->ClearContent();
			}
			CameraShake::StopAllCameraShake();
#endif
			Collision::CollisionBoxes.clear();
			std::ifstream Input(FilePath, std::ios::in | std::ios::binary);
			Input.exceptions(std::ios::failbit | std::ios::badbit);
			std::vector<WorldObject> WorldObjects;
			Editor::IsInSubscene = false;
			CurrentScene = FileUtil::GetFilePathWithoutExtension(FilePath);

			if (std::filesystem::is_empty(FilePath))
			{
				Graphics::WorldSun = Graphics::Sun();
				Graphics::WorldFog = Graphics::Fog();

				Log::Print("Loaded Scene (Scene File is empty)");
				return;
			}
#if SAVE_FOG_AND_SUN
			// Read global scene properties
			Vector3 SunProperties[4];
			Vector3 FogProperties[2];
			Input.read((char*)&SunProperties, sizeof(SunProperties));
			Input.read((char*)&FogProperties, sizeof(FogProperties));
			// Read the sun's properties
			Graphics::WorldSun.Rotation = SunProperties[0];
			Graphics::WorldSun.SunColor = SunProperties[1];
			Graphics::WorldSun.AmbientColor = SunProperties[2];
			Graphics::WorldSun.Intensity = SunProperties[3].X;
			Graphics::WorldSun.AmbientIntensity = SunProperties[3].Y;


			// Read the fog's properties
			Graphics::WorldFog.FogColor = FogProperties[0];
			Graphics::WorldFog.Falloff = FogProperties[1].X; 
			Graphics::WorldFog.Distance = FogProperties[1].Y;
			Graphics::WorldFog.MaxDensity = FogProperties[1].Z;
#endif
			std::string CubeName = ReadBinaryStringFromFile(Input);
#if !SERVER
			if (Graphics::MainFramebuffer)
			{
				Graphics::MainFramebuffer->ReflectionCubemapName = CubeName;
			}
#endif
			int ObjectLength = 0;
			Input.read((char*)&ObjectLength, sizeof(int));

			for (int i = 0; i < ObjectLength; i++)
			{
				if (!Input.eof())
				{
					Transform Transform1;
					Input.read((char*)&Transform1, sizeof(Transform));
					uint32_t ID = 0;
					Input.read((char*)&ID, sizeof(uint32_t));
					std::string Name;
					std::string Path;
					std::string desc;
					Name = ReadBinaryStringFromFile(Input);
					Path = ReadBinaryStringFromFile(Input);
					desc = ReadBinaryStringFromFile(Input);
					WorldObject* NewObject = Objects::SpawnObjectFromID(ID, Transform1);
					if (NewObject)
					{
						NewObject->Deserialize(Path);
						NewObject->Name = Name;
						NewObject->LoadProperties(desc);
						NewObject->OnPropertySet();
						NewObject->CurrentScene = CurrentScene;
					}
				}
			}

#if !SERVER
			BakedLighting::LoadBakeFile(FileUtil::GetFileNameWithoutExtensionFromPath(FilePath));
#endif
			Log::Print(std::string("Loaded Scene (").append(std::to_string(ObjectLength)).append(std::string(" Object(s) Loaded)")));
			Input.close();
		}
		else
		{
			Log::Print("Scene Loading Error: Scene \"" + OriginalString + "\" does not exist");
		}
	}

	void SaveSceneAs(std::string FilePath, bool Subscene)
	{
		Debugging::EngineStatus = "Saving Scene";
		std::ofstream Output(FilePath + (Subscene ? ".subscn" : ".jscn"), std::ios::out | std::ios::binary);
		std::vector<WorldObject*> SavedObjects;
		for (WorldObject* o : Objects::AllObjects)
		{
			if (o->CurrentScene == CurrentScene)
			{
				SavedObjects.push_back(o);
			}
		}
#if SAVE_FOG_AND_SUN
		if (!Subscene)
		{
			// Save global scene properties
			Vector3 SunProperties[4] =
			{
				Graphics::WorldSun.Rotation,
				Graphics::WorldSun.SunColor,
				Graphics::WorldSun.AmbientColor,
				Vector3(Graphics::WorldSun.Intensity, Graphics::WorldSun.AmbientIntensity, 0)
			};
			Vector3 FogProperties[2] =
			{
				Graphics::WorldFog.FogColor,
				Vector3(Graphics::WorldFog.Falloff, Graphics::WorldFog.Distance, Graphics::WorldFog.MaxDensity)
			};
			Output.write((char*)&SunProperties, sizeof(SunProperties));
			Output.write((char*)&FogProperties, sizeof(FogProperties));
		}
#endif
#if !SERVER
		WriteBinaryStringToFile(Graphics::MainFramebuffer->ReflectionCubemapName, Output);
#endif
		int ObjectLength = (int)SavedObjects.size();
		Output.write((char*)&ObjectLength, sizeof(int));

		for (int i = 0; i < ObjectLength; i++)
		{
			std::string name = SavedObjects.at(i)->Name;
			std::string path = SavedObjects.at(i)->Serialize();
			uint32_t ID = SavedObjects[i]->GetObjectDescription().ID; //C++ 20 or not?
			Transform T = SavedObjects[i]->GetTransform();
			Output.write((char*)&T, sizeof(Transform));
			Output.write((char*)&ID, sizeof(uint32_t));
			WriteBinaryStringToFile(name, Output);
			WriteBinaryStringToFile(path, Output);
			std::string Desc = SavedObjects[i]->GetPropertiesAsString();
			WriteBinaryStringToFile(Desc, Output);
		}

		Output.close();
	}
	void LoadSubScene(std::string FilePath)
	{
		FilePath = Assets::GetAsset(FilePath + ".subscn");
		for (int i = 0; i < Objects::AllObjects.size(); i++)
		{
			Objects::AllObjects.at(i)->IsSelected = false;
		}
		Editor::IsInSubscene = true;
		TextInput::PollForText = false;
		Debugging::EngineStatus = "Loading Subscene";
		if (std::filesystem::exists(FilePath))
		{
			std::ifstream Input(FilePath, std::ios::in | std::ios::binary);
			std::vector<WorldObject> WorldObjects;
			CurrentScene = FileUtil::GetFilePathWithoutExtension(FilePath);

			if (std::filesystem::is_empty(FilePath))
			{
				Log::Print("Loaded Subscene (Scene File is empty)");
				return;
			}
			int ObjectLength = 0;
			Input.read((char*)&ObjectLength, sizeof(int));

			for (int i = 0; i < ObjectLength; i++)
			{
				if (!Input.eof())
				{
					Transform Transform1;
					Input.read((char*)&Transform1, sizeof(Transform));
					uint32_t ID = 0;
					Input.read((char*)&ID, sizeof(uint32_t));
					std::string Name = ReadBinaryStringFromFile(Input);
					std::string Path = ReadBinaryStringFromFile(Input);
					std::string desc = ReadBinaryStringFromFile(Input);
					WorldObject* NewObject = Objects::SpawnObjectFromID(ID, Transform1);
					if (NewObject)
					{
						NewObject->Deserialize(Path);
						NewObject->Name = Name;
						NewObject->LoadProperties(desc);
						NewObject->OnPropertySet();
						NewObject->CurrentScene = CurrentScene;
					}
				}
			}
			Log::Print(std::string("Loaded Subscene (").append(std::to_string(ObjectLength)).append(std::string(" Object(s) Loaded)")));
			Input.close();
		}
	}
	void Tick()
	{
		if (ShouldLoadNewScene)
		{
			LoadSceneInternally(NewLoadedScene);
			ShouldLoadNewScene = false;
		}
	}
	void LoadNewScene(std::string FilePath)
	{
		NewLoadedScene = FilePath;
		ShouldLoadNewScene = true;
	}
}