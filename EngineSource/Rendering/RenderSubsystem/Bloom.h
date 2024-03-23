#pragma once
#include "RenderSubsystem.h"

struct Shader;

class Bloom : public RenderSubsystem
{
public:
	static unsigned int BlurFramebuffer(unsigned int buf);
	Bloom();
	void OnRendererResized() override;

private:
	static Shader* BloomShader;
	static unsigned int pingpongFBO[2];
	static unsigned int pingpongBuffer[2];
	static float BloomResolutionMultiplier;
	static int BloomShape;
};