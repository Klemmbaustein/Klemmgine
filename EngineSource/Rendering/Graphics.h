#pragma once
#include <Math/Vector.h>
#include <Engine/Subsystem/Subsystem.h>

class Renderable;
class UICanvas;
class Camera;
struct Shader;
class ScrollObject;
class FramebufferObject;

class Graphics : public Subsystem
{
public:
	Graphics();

	void Update() override;

	static float ResolutionScale;
	static bool RenderShadows;
	static bool SSAO;
	static bool VSync;
	static bool IsWireframe;
	static int ShadowResolution;
	static bool Bloom;
	static bool RenderFullBright;
	static float Gamma;

	static const int MAX_LIGHTS;

	struct Fog
	{
		float Distance = 70.f;
		float Falloff = 500.f;
		float MaxDensity = 1.f;
		Vector3 FogColor = Vector3(0.8f, 0.9f, 1.f);
	};
	struct Sun
	{
		float Intensity = 1.5f;
		float AmbientIntensity = 0.3f;
		Vector3 Rotation = Vector3(45.0f, 0.0f, 0.0f);
		Vector3 SunColor = Vector3(1.0f, 1.0f, 0.9f);
		Vector3 AmbientColor = Vector3(0.7f, 0.7f, 1.f);
	};

	struct Light
	{
		Light()
		{

		}
		Light(Vector3 Position, Vector3 Color, float Intensity, float Falloff)
		{
			this->Position = Position;
			this->Intensity = Intensity;
			this->Falloff = Falloff;
			this->Color = Color;
		}

		Vector3 Position = 0.f;
		float Intensity = 1.f;
		float Falloff = 20.f;
		float Distance = 0;
		Vector3 Color = 1.f;

		bool operator==(Light b);
	};

	static Sun WorldSun;
	static Fog WorldFog;
	static bool RenderAntiAlias;
	static std::vector<UICanvas*> UIToRender;
	static Vector2 WindowResolution;
	static Vector2 RenderResolution;
	static void SetWindowResolution(Vector2 NewResolution, bool Force = false);
	static float AspectRatio;
#if !SERVER
	static Camera* MainCamera;
	static Shader* MainShader;
	static Shader* TextShader;
	static Shader* UIShader;
#endif
	static float ChrAbbSize, Vignette;
	static unsigned int PCFQuality;
#if !SERVER
	static FramebufferObject* MainFramebuffer;
	static std::vector<FramebufferObject*> AllFramebuffers;
#endif
};