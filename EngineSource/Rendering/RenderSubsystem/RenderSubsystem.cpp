#include "RenderSubsystem.h"
#include <Rendering/RenderSubsystem/OcclusionCulling.h>
#include <Rendering/RenderSubsystem/PostProcess.h>
#include <Rendering/RenderSubsystem/SSAO.h>
#include <Rendering/RenderSubsystem/Bloom.h>
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Rendering/RenderSubsystem/CSM.h>
#include <Rendering/Graphics.h>

RenderSubsystem::RenderSubsystem()
{
	SystemType = "Renderer";
}

void RenderSubsystem::OnRendererResized()
{
}

void RenderSubsystem::ResizeAll()
{
	for (Subsystem* i : Subsystem::LoadedSystems)
	{
		if (dynamic_cast<RenderSubsystem*>(i))
		{
			static_cast<RenderSubsystem*>(i)->OnRendererResized();
		}
	}
}

void RenderSubsystem::LoadRenderSubsystems()
{
	Subsystem::Load(new Graphics());
	Subsystem::Load(new Bloom());
	Subsystem::Load(new CSM());
	Subsystem::Load(new OcclusionCulling());
	Subsystem::Load(new SSAO());
	Subsystem::Load(new PostProcess());
	Subsystem::Load(new BakedLighting());
}
