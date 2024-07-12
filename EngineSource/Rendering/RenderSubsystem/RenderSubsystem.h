#pragma once
#include <Engine/Subsystem/Subsystem.h>

class RenderSubsystem : public Subsystem
{
public:
	RenderSubsystem(bool IsDerived = false);
	~RenderSubsystem();

	virtual void OnRendererResized();

	static void ResizeAll();
};