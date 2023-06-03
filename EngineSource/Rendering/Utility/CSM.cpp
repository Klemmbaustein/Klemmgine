#include "CSM.h"
#include <Rendering/Camera/Camera.h>
#include <World/Graphics.h>
#include <iostream>
#include <cmath>
#include <Engine/Log.h>
#include <Engine/Console.h>
#include <GL/glew.h>

namespace CSM
{
	float CSMDistance = 1;
	const float cameraFarPlane = 400.f;
	std::vector<float> shadowCascadeLevels{ cameraFarPlane / 12.f, cameraFarPlane / 3.f, cameraFarPlane / 1.5f, cameraFarPlane / 1.f };
	GLuint LightFBO;
	int Cascades = 4;
	GLuint ShadowMaps;
	unsigned int matricesUBO;
	bool Initialized = false;
	std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
	{
		const auto inv = glm::inverse(projview);

		std::vector<glm::vec4> frustumCorners;
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
					frustumCorners.push_back(pt / pt.w);
				}
			}
		}

		return frustumCorners;
	}

	std::string ErrorMessageFromGLStatus(int Status)
	{
		//https://neslib.github.io/Ooogles.Net/html/0e1349ae-da69-6e5e-edd6-edd8523101f8.htm

		switch (Status)
		{
		case 36053:
			return "The framebuffer is complete\n";
		case 36054:
			return "Not all framebuffer attachment points are framebuffer attachment complete.\nThis means that at least one attachment point with a renderbuffer or texture attached has its attached object no longer in existence \nor has an attached image with a width or height of zero, or the color attachment point has a non-color-renderable image attached, \nor the depth attachment point has a non-depth-renderable image attached, or the stencil attachment point \nhas a non-stencil-renderable image attached.\n";
		case 36057:
			return "Not all attached images have the same width and height.";
		case 36055:
			return "No images are attached to the framebuffer.";
		case 36061:
			return "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
		}
		return "Error: The given Status is not A glFramebufferError";
	}

	void UpdateMatricesUBO()
	{
		if (!Graphics::RenderShadows || Graphics::ShadowResolution <= 0)
		{
			return;
		}
		const auto LightSpaceMatrices = CSM::getLightSpaceMatrices();

		glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
		for (size_t i = 0; i < LightSpaceMatrices.size(); ++i)
		{
			glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &LightSpaceMatrices[i]);
		}
	}

	void BindLightSpaceMatricesToShader(const std::vector<glm::mat4>& Matrices, Shader* ShaderToBind)
	{
		if (!Graphics::RenderShadows || Graphics::ShadowResolution <= 0)
		{
			return;
		}
		ShaderToBind->Bind();
		for (size_t i = 0; i < Matrices.size(); ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(ShaderToBind->GetShaderID(),
				((std::string("lightSpaceMatrices[") + std::to_string(i)) + "]").c_str()), 1, GL_FALSE, &Matrices.at(i)[0][0]);
		}
	}

	std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
	{
		return getFrustumCornersWorldSpace(proj * view);
	}

	void Init()
	{
		Initialized = true;
		glGenFramebuffers(1, &LightFBO);

		glGenTextures(1, &ShadowMaps);
		glBindTexture(GL_TEXTURE_2D_ARRAY, ShadowMaps);
		glTexImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			GL_DEPTH_COMPONENT32F,
			Graphics::ShadowResolution,
			Graphics::ShadowResolution,
			int(shadowCascadeLevels.size()) + 1,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			nullptr);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

		glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ShadowMaps, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
			throw 0;
		}
		glGenBuffers(1, &matricesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 8, nullptr, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		Console::RegisterConVar(Console::Variable("shadow_resolution", Type::E_INT, &Graphics::ShadowResolution, CSM::ReInit));
		Console::RegisterConVar(Console::Variable("shadows", Type::E_BOOL, &Graphics::RenderShadows, CSM::ReInit));
		Console::RegisterConVar(Console::Variable("shadow_distance", Type::E_FLOAT, &CSMDistance, nullptr));
	}

	void ReInit()
	{
		if (!Initialized)
		{
			return;
		}
		if (ShadowMaps)
		{
			glDeleteTextures(1, &ShadowMaps);
			glDeleteFramebuffers(1, &LightFBO);
			glGenFramebuffers(1, &LightFBO);
			ShadowMaps = 0;
		}
		
		if (Graphics::RenderShadows && Graphics::ShadowResolution > 0)
		{
			glGenTextures(1, &ShadowMaps);
			glBindTexture(GL_TEXTURE_2D_ARRAY, ShadowMaps);
			glTexImage3D(
				GL_TEXTURE_2D_ARRAY,
				0,
				GL_DEPTH_COMPONENT32F,
				Graphics::ShadowResolution,
				Graphics::ShadowResolution,
				int(shadowCascadeLevels.size()) + 1,
				0,
				GL_DEPTH_COMPONENT,
				GL_FLOAT,
				nullptr);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

			glBindFramebuffer(GL_FRAMEBUFFER, LightFBO);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ShadowMaps, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);

			int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
				throw 0;
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane)
	{
		const auto proj = glm::perspective(
			2.f, (float)Graphics::WindowResolution.X / (float)Graphics::WindowResolution.Y, nearPlane,
			farPlane);

		const auto corners = getFrustumCornersWorldSpace(proj, Graphics::MainCamera->getView());

		glm::vec3 center = glm::vec3(0, 0, 0);
		for (const auto& v : corners)
		{
			center += glm::vec3(v);
		}
		center /= corners.size();
		float SnapSize = farPlane / 10;

		center = Vector3::SnapToGrid(center, SnapSize);
		auto lightView = glm::lookAt(center + (glm::vec3)Graphics::WorldSun.Direction, center, glm::vec3(0.0f, 1.0f, 0.0f));

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::min();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::min();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::min();
		for (const auto& v : corners)
		{
			const auto trf = lightView * v;
			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		minX = std::floor(minX / SnapSize) * SnapSize;
		minY = std::floor(minY / SnapSize) * SnapSize;
		minZ = std::floor(minZ / SnapSize) * SnapSize;
		maxX = std::floor(maxX / SnapSize) * SnapSize;
		maxY = std::floor(maxY / SnapSize) * SnapSize;
		maxZ = std::floor(maxZ / SnapSize) * SnapSize;

		// Tune this parameter according to the scene
		constexpr float zMult = 25;
		if (minZ < 0)
		{
			minZ *= zMult;
		}
		else
		{
			minZ /= zMult;
		}
		if (maxZ < 0)
		{
			maxZ /= zMult;
		}
		else
		{
			maxZ *= zMult;
		}

		float zoom = 0.8;

		const glm::mat4 lightProjection = glm::ortho(minX / zoom, maxX / zoom, minY / zoom, maxY / zoom, minZ / zoom, maxZ / zoom);

		return lightProjection * lightView;
	}

	std::vector<glm::mat4> getLightSpaceMatrices()
	{
		std::vector<glm::mat4> ret;
		for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
		{
			if (i == 0)
			{
				ret.push_back(getLightSpaceMatrix(0.1f * CSMDistance, shadowCascadeLevels[i] * CSMDistance));
			}
			else if (i < shadowCascadeLevels.size())
			{
				ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1] * CSMDistance, shadowCascadeLevels[i] * CSMDistance));
			}
			else
			{
				ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1] * CSMDistance, cameraFarPlane * CSMDistance));
			}
		}
		return ret;
	}
}