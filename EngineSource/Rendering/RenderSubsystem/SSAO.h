#if !SERVER
#include "RenderSubsystem.h"

#pragma once
class SSAO : public RenderSubsystem
{
public:
	SSAO();
	static unsigned int Render(unsigned int NormalBuffer, unsigned int PositionBuffer);
	void OnRendererResized() override;
};
#endif