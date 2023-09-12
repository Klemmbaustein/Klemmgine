#include "PostProcess.h"
#include <Rendering/Shader.h>
#include <GL/glew.h>
#include <Rendering/Graphics.h>
#include <Engine/Stats.h>

namespace PostProcess
{
	std::vector<Effect*> AllEffects;
}

unsigned int PostProcess::Effect::Render(unsigned int TargetBuffer) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, EffectBuffer);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	EffectShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TargetBuffer);
	EffectShader->SetInt("u_texture", 0);
	EffectShader->SetFloat("u_time", Stats::Time);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 3);


	return EffectTexture;
}

PostProcess::Effect::Effect(std::string FragmentShader, EffectType UsedType)
{
	this->UsedType = UsedType;
	EffectShader = new Shader("Shaders/Internal/postprocess.vert", "Shaders/" + FragmentShader);
	glGenFramebuffers(1, &EffectBuffer);
	glGenTextures(1, &EffectTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, EffectBuffer);
	glBindTexture(GL_TEXTURE_2D, EffectTexture);
	int SizeX = (int)(Graphics::WindowResolution.X), SizeY = (int)(Graphics::WindowResolution.Y);

	if (UsedType == EffectType::UI || UsedType == EffectType::UI_Internal)
	{
		SizeX *= 2;
		SizeY *= 2;
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		SizeX,
		SizeY,
		0,
		GL_RGBA,
		GL_FLOAT,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, EffectTexture, 0
	);
}

void PostProcess::Effect::UpdateSize()
{
	glDeleteFramebuffers(1, &EffectBuffer);
	glDeleteTextures(1, &EffectTexture);
	glGenFramebuffers(1, &EffectBuffer);
	glGenTextures(1, &EffectTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, EffectBuffer);
	glBindTexture(GL_TEXTURE_2D, EffectTexture);

	int SizeX = (int)(Graphics::WindowResolution.X), SizeY = (int)(Graphics::WindowResolution.Y);

	if (UsedType == EffectType::UI || UsedType == EffectType::UI_Internal)
	{
		SizeX *= 2;
		SizeY *= 2;
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		SizeX,
		SizeY,
		0,
		GL_RGBA,
		GL_FLOAT,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, EffectTexture, 0
	);
}

void PostProcess::AddEffect(Effect* NewEffect)
{
	AllEffects.push_back(NewEffect);
}

bool PostProcess::RemoveEffect(Effect*& TargetEffect)
{
	for (size_t i = 0; i < AllEffects.size(); i++)
	{
		if (AllEffects[i] == TargetEffect)
		{
			AllEffects.erase(AllEffects.begin() + i);
			delete TargetEffect;
			TargetEffect = nullptr;
			return true;
		}
	}
	return false;
}

const std::vector<PostProcess::Effect*> PostProcess::GetCurrentEffects()
{
	return PostProcess::AllEffects;
}
