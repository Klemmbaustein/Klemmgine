#pragma once
#include <Rendering/Camera/Camera.h>
#include <Rendering/Shader.h>
class Renderable
{
public:
	virtual void Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass) = 0;
	virtual void SimpleRender(Shader* UsedShader) = 0;
	Renderable()
	{

	};
	virtual ~Renderable()
	{

	};

	static void ApplyDefaultUniformsToShader(Shader* ShaderToApply);

	bool CastShadow = true;
};