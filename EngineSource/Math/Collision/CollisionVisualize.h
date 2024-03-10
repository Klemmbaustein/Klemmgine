#pragma once
#if !SERVER

class FramebufferObject;

namespace CollisionVisualize
{
	void Activate();
	void Deactivate();

	void Update();

	FramebufferObject* GetVisualizeBuffer();

	bool GetIsActive();
}

#endif