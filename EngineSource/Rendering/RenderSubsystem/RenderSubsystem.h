#pragma once
#include <Engine/Subsystem/Subsystem.h>

class RenderSubsystem : public Subsystem
{
public:
	RenderSubsystem();
	virtual void OnRendererResized();

	static void ResizeAll();

	static void LoadRenderSubsystems();
};