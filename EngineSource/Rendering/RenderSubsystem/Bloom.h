#pragma once
#include "RenderSubsystem.h"

struct Shader;

/**
* @brief
* Bloom subsystem.
* 
* @ingroup Subsystem
* @ingroup Internal
*/
class Bloom : public RenderSubsystem
{
public:
	static unsigned int BlurFramebuffer(unsigned int buf);
	Bloom();
	void OnRendererResized() override;

private:
	static Shader* BloomShader;
	static unsigned int PingPongFBO[2];
	static unsigned int PingPongBuffer[2];
	static float BloomResolutionMultiplier;
	static int BloomShape;
};