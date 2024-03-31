#include "Graphics.h"
#include <UI/UIBox.h>
#include <Rendering/Framebuffer.h>
#include "RenderSubsystem/PostProcess.h"
#include "RenderSubsystem/RenderSubsystem.h"
#include <Engine/Subsystem/Scene.h>

float Graphics::ResolutionScale = 1.0f;
bool Graphics::RenderShadows = true;
bool Graphics::SSAO = true;
bool Graphics::VSync = true;
bool Graphics::Bloom = true;
bool Graphics::IsWireframe = false;
bool Graphics::RenderAntiAlias = true;
bool Graphics::RenderFullbright = false;
Graphics::Sun Graphics::WorldSun;
Graphics::Fog Graphics::WorldFog;
const int Graphics::MAX_LIGHTS = 8;
int Graphics::ShadowResolution = 2000;
std::vector<UICanvas*> Graphics::UIToRender;
Vector2 Graphics::WindowResolution(1600, 900);
Vector2 Graphics::RenderResolution = WindowResolution;
unsigned int Graphics::PCFQuality = 0;
float Graphics::AspectRatio = 16.0f / 9.0f;

Graphics::Graphics()
{
	Name = "Graphics";
#if !SERVER
	Graphics::MainCamera = Scene::DefaultCamera;
	Graphics::MainFramebuffer = new FramebufferObject();
	Graphics::MainFramebuffer->FramebufferCamera = Graphics::MainCamera;
#endif
}

void Graphics::Update()
{

}

void Graphics::SetWindowResolution(Vector2 NewResolution, bool Force)
{
#if !SERVER
	if (NewResolution * ResolutionScale == RenderResolution && !Force)
	{
		return;
	}

	Graphics::MainCamera->ReInit(Graphics::MainCamera->FOV, NewResolution.X, NewResolution.Y, false);
	AspectRatio = NewResolution.X / NewResolution.Y;
	WindowResolution = NewResolution;
	RenderResolution = NewResolution * ResolutionScale;
	RenderSubsystem::ResizeAll();

	for (FramebufferObject* o : AllFramebuffers)
	{
		if (o->UseMainWindowResolution)
		{
			o->GetBuffer()->ReInit((unsigned int)RenderResolution.X, (unsigned int)(RenderResolution.Y));
		}
	}

	for (PostProcess::Effect* i : PostProcess::GetCurrentEffects())
	{
		i->UpdateSize();
	}

	UIBox::ForceUpdateUI();
#endif
}

float Graphics::Gamma = 1;
float Graphics::ChrAbbSize = 0, Graphics::Vignette = 0.1f;
#if !SERVER

Camera* Graphics::MainCamera;
Shader* Graphics::MainShader;
Shader* Graphics::TextShader;
Shader* Graphics::UIShader;

#endif

bool CanRenderText = true;

#if !SERVER

FramebufferObject* Graphics::MainFramebuffer;
std::vector<FramebufferObject*> Graphics::AllFramebuffers;
bool Graphics::Light::operator==(Light b)
{
	return Position == b.Position && Color == b.Color && Intensity == b.Intensity && Falloff == b.Falloff;
}

#endif
