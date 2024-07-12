#if !SERVER
#include "RenderSubsystem.h"
#include <GL/glew.h>
#include <Rendering/RenderSubsystem/OcclusionCulling.h>
#include <Rendering/RenderSubsystem/PostProcess.h>
#include <Rendering/RenderSubsystem/SSAO.h>
#include <Rendering/RenderSubsystem/Bloom.h>
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Rendering/RenderSubsystem/CSM.h>
#include <Rendering/Graphics.h>
#include <Engine/Stats.h>
#include <Engine/Subsystem/Console.h>
#include <Engine/AppWindow.h>

static void GLAPIENTRY MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	if (type == GL_DEBUG_TYPE_ERROR
		|| type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
		|| type == GL_DEBUG_TYPE_PORTABILITY)
	{
		Subsystem::GetSubsystemByName("Renderer")->Print(std::string(message) + " - " + Stats::EngineStatus, Subsystem::ErrorLevel::Error);
	}
}

RenderSubsystem::RenderSubsystem(bool IsDerived)
{
	SystemType = "Renderer";

	if (!IsDerived)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
		Subsystem::Load(new Graphics());
		Subsystem::Load(new Bloom());
		Subsystem::Load(new CSM());
		Subsystem::Load(new OcclusionCulling());
		Subsystem::Load(new SSAO());
		Subsystem::Load(new PostProcess());
		Subsystem::Load(new BakedLighting());

		Console::ConsoleSystem->RegisterConVar(Console::Variable("wireframe", NativeType::Bool, &Graphics::IsWireframe, nullptr));
		Console::ConsoleSystem->RegisterConVar(Console::Variable("vignette", NativeType::Float, &Graphics::Vignette, nullptr));
		Console::ConsoleSystem->RegisterConVar(Console::Variable("vsync", NativeType::Bool, &Graphics::VSync, nullptr));
		Console::ConsoleSystem->RegisterConVar(Console::Variable("timescale", NativeType::Float, &Stats::TimeMultiplier, nullptr));
		Console::ConsoleSystem->RegisterConVar(Console::Variable("resolution_scale", NativeType::Float, &Graphics::ResolutionScale, []()
			{
				Graphics::SetWindowResolution(Window::GetWindowSize());
			}));
	}
}

RenderSubsystem::~RenderSubsystem()
{
	glDisable(GL_DEBUG_OUTPUT);
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

#endif