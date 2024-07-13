#if !SERVER
#pragma once
#include "RenderSubsystem.h"

/**
* @brief
* Subsystem for screen space ambient occlusion.
* 
* @ingroup Internal
* @ingroup Subsystem
*/
class SSAO : public RenderSubsystem
{
public:
	SSAO();
	static unsigned int Render(unsigned int NormalBuffer, unsigned int PositionBuffer);
	void OnRendererResized() override;
};
#endif