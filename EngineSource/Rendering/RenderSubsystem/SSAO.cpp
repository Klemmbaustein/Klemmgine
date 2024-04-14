#if !SERVER
#include "SSAO.h"
#include <random>
#include <vector>
#include <glm/glm.hpp>
#include <Rendering/ShaderManager.h>
#include <Rendering/Camera/Camera.h>
#include <Rendering/Graphics.h>
#include <GL/glew.h>
#include <cmath>
#include <Engine/Subsystem/Console.h>

// https://learnopengl.com/Advanced-Lighting/SSAO

namespace SSAO_Impl
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;
	Shader* AOShader = nullptr;
	unsigned int noiseTexture = 0;
	unsigned int ssaoFBO = 0;
	unsigned int ssaoColorBuffer = 0;
	unsigned int ssaoBlurFBO = 0, ssaoColorBufferBlur = 0;
	float ResolutionDivider = 1.5f;
	unsigned int Samples = 12;
	unsigned int SSAOTexture = 0;
}

SSAO::SSAO()
{
	using namespace SSAO_Impl;
	Name = "SSAO";
	for (unsigned int i = 0; i < Samples; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / Samples;
		scale = std::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}
	for (unsigned int i = 0; i < 16; ++i)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	OnRendererResized();
	AOShader = new Shader("Shaders/Internal/postprocess.vert", "Shaders/Internal/ssao.frag");
	Console::ConsoleSystem->RegisterConVar(Console::Variable("ssao", NativeType::Bool, &Graphics::SSAO, nullptr));
}

unsigned int SSAO::Render(unsigned int NormalBuffer, unsigned int PositionBuffer)
{
	using namespace SSAO_Impl;
	if (!Graphics::MainCamera) return 0;
	if (!Graphics::SSAO) return 0;
	glViewport(0, 0, (int)(Graphics::RenderResolution.X / ResolutionDivider), (int)(Graphics::RenderResolution.Y / ResolutionDivider));
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, PositionBuffer);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalBuffer);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	AOShader->Bind();
	glUniform1i(glGetUniformLocation(AOShader->GetShaderID(), "FullScreen"), 1);
	glUniform1i(glGetUniformLocation(AOShader->GetShaderID(), "kernelSize"), Samples);
	glUniform1f(glGetUniformLocation(AOShader->GetShaderID(), "radius"), 15);
	glUniform1i(glGetUniformLocation(AOShader->GetShaderID(), "gPosition"), 0);
	glUniform1i(glGetUniformLocation(AOShader->GetShaderID(), "gNormal"), 1);
	glUniform1f(glGetUniformLocation(AOShader->GetShaderID(), "ResDiv"), ResolutionDivider);
	glUniform1i(glGetUniformLocation(AOShader->GetShaderID(), "texNoise"), 2);
	glUniform2f(glGetUniformLocation(AOShader->GetShaderID(), "screenRes"), Graphics::RenderResolution.X, Graphics::RenderResolution.Y);
	for (unsigned int i = 0; i < Samples; ++i)
		glUniform3fv(glGetUniformLocation(AOShader->GetShaderID(), ("samples[" + std::to_string(i) + "]").c_str()), 1, &ssaoKernel[i].x);
	glUniformMatrix4fv(glGetUniformLocation(AOShader->GetShaderID(), "projection"), 1, false, &Graphics::MainCamera->GetProjection()[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, (GLsizei)Graphics::RenderResolution.X, (GLsizei)Graphics::RenderResolution.Y);
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);

	return ssaoColorBuffer;
}

void SSAO::OnRendererResized()
{
	using namespace SSAO_Impl;
	if (ssaoColorBuffer)
	{
		glDeleteTextures(1, &ssaoColorBuffer);
		glDeleteFramebuffers(1, &ssaoFBO);
	}
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RG,
		(GLsizei)(Graphics::RenderResolution.X / ResolutionDivider),
		(GLsizei)(Graphics::RenderResolution.Y / ResolutionDivider),
		0,
		GL_RED,
		GL_FLOAT,
		NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

}
#endif