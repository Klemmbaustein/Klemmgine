#include <World/Assets.h>
#include <World/Graphics.h>
#include <World/Stats.h>

#include <Rendering/Utility/Bloom.h>
#include <Rendering/Utility/SSAO.h>
#include <filesystem>
#include <Objects/Components/CollisionComponent.h>
#include <Engine/EngineProperties.h>
#include <UI/Default/ScrollObject.h>
#include <Engine/FileUtility.h>
#include <Rendering/Utility/Framebuffer.h>
#include <UI/UIBox.h>
#include <Engine/OS.h>
#include <map>
#include <Engine/Log.h>

namespace Engine
{
	std::string CurrentProjectName = ProjectName;
	std::string VersionString = VERSION_STRING + std::string(IS_IN_EDITOR ? "-Editor" : "-Build");
}
namespace Graphics
{
	bool RenderShadows = true;
	bool SSAO = true;
	bool VSync = true;
	bool Bloom = true, FXAA = false;
	bool IsWireframe = false; 

	std::vector<Light> Lights;
	Sun WorldSun;
	Fog WorldFog;
	int ShadowResolution = 2000;
	std::vector<Renderable*> ModelsToRender;
	std::vector<UICanvas*> UIToRender;
	Vector2 WindowResolution(1600, 900);
	unsigned int PCFQuality = 0;
	float AspectRatio = 16.0 / 9.0;
	void SetWindowResolution(Vector2 NewResolution)
	{
		if (NewResolution == WindowResolution)
		{
			return;
		}
		for (FramebufferObject* o : AllFramebuffers)
		{
			if (o->UseMainWindowResolution)
			{
				o->GetBuffer()->ReInit(NewResolution.X, NewResolution.Y);
			}
		}
		Graphics::MainCamera->ReInit(Graphics::MainCamera->FOV, NewResolution.X, NewResolution.Y, false);
		AspectRatio = NewResolution.X / NewResolution.Y;
		WindowResolution = NewResolution;
		SSAO::ResizeBuffer(NewResolution.X, NewResolution.Y);
		Bloom::OnResized();
		UIBox::ForceUpdateUI();
	}
	float Gamma = 1;
	float ChrAbbSize = 0, Vignette = 0.2;
	Camera* MainCamera;
	Shader* MainShader;
	Shader* ShadowShader;
	Shader* TextShader;
	Shader* UIShader;
	std::vector<CollisionComponent*> Objects;
	bool IsRenderingShadows = false;
	namespace UI
	{
		std::vector<ScrollObject*> ScrollObjects;
	}
	bool CanRenderText = true;
	namespace FBO
	{
		unsigned int SSAOBuffers[3];
		unsigned int ssaoColorBuffer;
		unsigned int ssaoFBO;
	}
	FramebufferObject* MainFramebuffer; 
	std::vector<FramebufferObject*> AllFramebuffers;
}


const bool IsInEditor = IS_IN_EDITOR;
const bool EngineDebug = ENGINE_DEBUG;

namespace Log
{
	std::map<OS::EConsoleColor, Vector3> ConsoleColors =
	{
		std::pair(OS::E_WHITE, Vector3(1)),
		std::pair(OS::E_GRAY, Vector3(0.5)),
		std::pair(OS::E_RED, Vector3(1, 0, 0)),
		std::pair(OS::E_GREEN, Vector3(0, 1, 0)),
		std::pair(OS::E_BLUE, Vector3(0, 0, 1)),
		std::pair(OS::E_YELLOW, Vector3(1, 1, 0)),
	};
	std::vector<Message> Messages;
	void Print(std::string Text, Vector3 Color)
	{
		if (Messages.size() > 0)
		{
			if (Messages.at(Messages.size() - 1).Text == Text && Messages.at(Messages.size() - 1).Color == Color)
			{
				Messages.at(Messages.size() - 1).Ammount++;
			}
			else
			{
				Messages.push_back(Message(Text, Color));
			}
		}
		else
		{
			Messages.push_back(Message(Text, Color));
		}
		OS::SetConsoleColor(OS::E_GRAY);
		OS::EConsoleColor NearestColor = OS::E_WHITE;
		for (const auto& c : ConsoleColors)
		{
			if (Vector3::NearlyEqual(c.second, Color, 0.35))
			{
				NearestColor = c.first;
			}
		}
		std::cout << "Log: ";
		OS::SetConsoleColor(NearestColor);
		std::cout << Text << "\n";
		OS::SetConsoleColor(OS::E_GRAY);
#if EDITOR
		UIBox::RedrawUI();
#endif
	}
}

namespace Collision
{
	std::vector<CollisionComponent*> CollisionBoxes;
}

namespace Performance
{
	float DeltaTime;
	float FPS;
	float TimeMultiplier = 1;
	unsigned int DrawCalls = 0;
}

namespace Objects
{
	std::vector<WorldObject*> AllObjects;
	std::vector<WorldObject*> GetAllObjectsWithID(uint32_t ID)
	{
		std::vector<WorldObject*> FoundObjects;
		for (WorldObject* o : Objects::AllObjects)
		{
			if (o->GetObjectDescription().ID == ID)
			{
				FoundObjects.push_back(o);
			}
		}
		return FoundObjects;
	}
}

namespace Stats
{
	float Time = 0;
}


namespace TextInput
{
	bool PollForText = false;
	std::string Text;
	int TextIndex = 0u;
}

namespace Debugging
{
	std::string EngineStatus;
}

namespace Assets
{
	std::vector<Asset> Assets;

	void ScanForAssets(std::string Path, bool Recursive)
	{
		if (!Recursive)
		{
			Assets.clear(); 
			if (!(IsInEditor || EngineDebug))
			{
				Path = "Assets/" + Path;
			}
		}
		for (const auto& entry : std::filesystem::directory_iterator(Path))
		{
			if (entry.is_directory())
			{
				ScanForAssets(entry.path().string(), true);
			}
			else
			{
				std::string Path = entry.path().string();
#if _WIN32 // Replace all backslashes with forward slashes for consistency.
				for (auto& i : Path)
				{
					if (i == '\\')
					{
						i = '/';
					}
				}
#endif
				Assets.push_back(Asset(Path, GetFileNameFromPath(Path)));
			}
		}
	}


	std::string GetAsset(std::string Name)
	{
		for (const Asset& s : Assets::Assets)
		{
			if (s.Name == Name)
			{
				return s.Filepath;
			}
		}
		return "";
	}
}

namespace Editor
{
	bool IsInSubscene = false;
}