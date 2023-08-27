#pragma once
#include <Math/Vector.h>

class Renderable;
class UICanvas;
class Camera;
struct Shader;
class ScrollObject;
class FramebufferObject;

namespace Graphics
{
	extern float ResolutionScale;
	extern bool RenderShadows;
	extern bool SSAO;
	extern bool VSync;
	extern bool IsWireframe;
	extern int ShadowResolution;
	extern bool Bloom, FXAA;
	extern float Gamma;

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
		Vector3 Direction = Vector3(0.5f, 0.5f, 0.7f);
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
		float Falloff = 250.f;
		Vector3 Color = 1.f;

		bool operator==(Light b);
	};

	extern Sun WorldSun;
	extern Fog WorldFog;
	extern std::vector<UICanvas*> UIToRender;
	extern Vector2 WindowResolution;
	void SetWindowResolution(Vector2 NewResolution);
	extern float AspectRatio;
	extern Camera* MainCamera;
	extern Shader* MainShader;
	extern Shader* ShadowShader;
	extern Shader* TextShader;
	extern bool IsRenderingShadows;
	extern float ChrAbbSize, Vignette;
	extern Shader* UIShader;
	extern unsigned int PCFQuality;
	extern FramebufferObject* MainFramebuffer;
	namespace UI
	{
		extern std::vector<ScrollObject*> ScrollObjects;
	}
	void RecompileShaders();
	namespace FBO
	{
		extern unsigned int SSAOBuffers[3];
		extern unsigned int ssaoColorBuffer;
		extern unsigned int ssaoFBO;
	}
	extern std::vector<FramebufferObject*> AllFramebuffers;
}