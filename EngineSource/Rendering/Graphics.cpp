#include "Graphics.h"
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Utility/Bloom.h>
#include <Rendering/Utility/SSAO.h>
#include <Rendering/Utility/CSM.h>
#include <UI/UIBox.h>
#include <Rendering/Utility/PostProcess.h>

namespace Graphics
{
	float ResolutionScale = 1.0f;
	bool RenderShadows = true;
	bool SSAO = true;
	bool VSync = true;
	bool Bloom = true;
	bool IsWireframe = false;
	bool RenderAntiAlias = true;
	bool RenderFullbright = false;
	Sun WorldSun;
	Fog WorldFog;
	int ShadowResolution = 2000;
	std::vector<UICanvas*> UIToRender;
	Vector2 WindowResolution(1600, 900);
	Vector2 RenderResolution = WindowResolution;
	unsigned int PCFQuality = 0;
	float AspectRatio = 16.0f / 9.0f;
	void SetWindowResolution(Vector2 NewResolution, bool Force)
	{
#if !SERVER
		if (NewResolution * ResolutionScale == RenderResolution && !Force)
		{
			return;
		}

		for (FramebufferObject* o : AllFramebuffers)
		{
			if (o->UseMainWindowResolution)
			{
				o->GetBuffer()->ReInit((unsigned int)(NewResolution.X * ResolutionScale), (int)(NewResolution.Y * ResolutionScale));
			}
		}
		Graphics::MainCamera->ReInit(Graphics::MainCamera->FOV, NewResolution.X, NewResolution.Y, false);
		AspectRatio = NewResolution.X / NewResolution.Y;
		WindowResolution = NewResolution;
		RenderResolution = NewResolution * ResolutionScale;
		SSAO::ResizeBuffer((unsigned int)(NewResolution.X * ResolutionScale), (unsigned int)(NewResolution.Y * ResolutionScale));
		Bloom::OnResized();
		for (PostProcess::Effect* i : PostProcess::GetCurrentEffects())
		{
			i->UpdateSize();
		}

		UIBox::ForceUpdateUI();
#endif
	}
	float Gamma = 1;
	float ChrAbbSize = 0, Vignette = 0.1f;
#if !SERVER
	Camera* MainCamera;
	Shader* MainShader;
	Shader* ShadowShader;
	Shader* TextShader;
	Shader* UIShader;
#endif
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
#if !SERVER
	FramebufferObject* MainFramebuffer;
	std::vector<FramebufferObject*> AllFramebuffers;
	bool Light::operator==(Light b)
	{
		return Position == b.Position && Color == b.Color && Intensity == b.Intensity && Falloff == b.Falloff;
	}
#endif
}