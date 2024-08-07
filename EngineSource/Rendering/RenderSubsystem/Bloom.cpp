#if !SERVER
#include "Bloom.h"
#include <GL/glew.h>
#include <SDL.h>
#include <Rendering/Framebuffer.h>
#include <Engine/Subsystem/Console.h>

#include <Rendering/Graphics.h>

float Bloom::BloomResolutionMultiplier = 0.15f;
int Bloom::BloomShape = 2;
Shader* Bloom::BloomShader = nullptr;
unsigned int Bloom::PingPongFBO[2];
unsigned int Bloom::PingPongBuffer[2];

unsigned int Bloom::BlurFramebuffer(unsigned int buf)
{
	if (Graphics::Bloom && BloomShape != 0)
	{
		glViewport(0,
			0,
			(unsigned int)(Graphics::WindowResolution.X * BloomResolutionMultiplier),
			(unsigned int)(Graphics::WindowResolution.Y * BloomResolutionMultiplier));
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 15u;
		BloomShader->Bind();
		glBindFramebuffer(GL_FRAMEBUFFER, PingPongFBO[horizontal]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUniform1i(glGetUniformLocation(BloomShader->GetShaderID(), "FullScreen"), 1);
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, PingPongFBO[horizontal]);
			glUniform1i(glGetUniformLocation(BloomShader->GetShaderID(), "horizontal"), (i % (unsigned int)BloomShape) != 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(
				GL_TEXTURE_2D, first_iteration ? buf : PingPongBuffer[!horizontal]
			);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, (unsigned int)Graphics::WindowResolution.X, (unsigned int)Graphics::WindowResolution.Y);

	}
	return PingPongBuffer[1];
}

Bloom::Bloom()
	: RenderSubsystem(true)
{
	Name = "Bloom";
	BloomShader = new Shader("Shaders/Internal/postprocess.vert", "Shaders/Internal/bloom.frag");
	glGenFramebuffers(2, PingPongFBO);
	glGenTextures(2, PingPongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, PingPongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, PingPongBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D,
			0, 
			GL_RGBA16F,
			(int)(Graphics::WindowResolution.X * BloomResolutionMultiplier),
			(int)(Graphics::WindowResolution.Y * BloomResolutionMultiplier),
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
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PingPongBuffer[i], 0
		);
	}
	Console::ConsoleSystem->RegisterConVar(Console::Variable("bloom", NativeType::Bool, &Graphics::Bloom, nullptr));
	Console::ConsoleSystem->RegisterConVar(Console::Variable("bloom_shape", NativeType::Int, &Bloom::BloomShape, nullptr));
}

void Bloom::OnRendererResized()
{
	glDeleteFramebuffers(2, PingPongFBO);
	glDeleteTextures(2, PingPongBuffer);
	glGenFramebuffers(2, PingPongFBO);
	glGenTextures(2, PingPongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, PingPongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, PingPongBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA16F,
			(int)(Graphics::WindowResolution.X * BloomResolutionMultiplier),
			(int)(Graphics::WindowResolution.Y * BloomResolutionMultiplier),
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
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PingPongBuffer[i], 0
		);
	}
}
#endif