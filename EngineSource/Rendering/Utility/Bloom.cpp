#include "Bloom.h"
#include <GL/glew.h>
#include <SDL.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Console.h>

#include <World/Graphics.h>

namespace Bloom
{
	Shader* BloomShader;
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];
	float BloomResolutionMultiplier = 0.15;
}

unsigned int Bloom::BlurFramebuffer(unsigned int buf)
{
	if (Graphics::Bloom)
	{
		glViewport(0, 0, Graphics::WindowResolution.X * BloomResolutionMultiplier, Graphics::WindowResolution.Y * BloomResolutionMultiplier);
		bool horizontal = true, first_iteration = true;
		int amount = 15;
		BloomShader->Bind();
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniform1i(glGetUniformLocation(BloomShader->GetShaderID(), "FullScreen"), 1);
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			glUniform1i(glGetUniformLocation(BloomShader->GetShaderID(), "horizontal"), horizontal);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(
				GL_TEXTURE_2D, first_iteration ? buf : pingpongBuffer[!horizontal]
			);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, Graphics::WindowResolution.X, Graphics::WindowResolution.Y);

	}
    return pingpongBuffer[1];
}

void Bloom::Init()
{
	BloomShader = new Shader("Shaders/Internal/postprocess.vert", "Shaders/Internal/bloom.frag");
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, Graphics::WindowResolution.X * BloomResolutionMultiplier, Graphics::WindowResolution.Y * BloomResolutionMultiplier, 0,
			GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
		);
	}
	Console::RegisterConVar(Console::Variable("bloom", Type::E_BOOL, &Graphics::Bloom, nullptr));
}

void Bloom::OnResized()
{
	glDeleteFramebuffers(2, pingpongFBO);
	glDeleteTextures(2, pingpongBuffer);
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, Graphics::WindowResolution.X * BloomResolutionMultiplier, Graphics::WindowResolution.Y * BloomResolutionMultiplier, 0,
			GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
		);
	}
}