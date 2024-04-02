#if !SERVER
#include "CSM.h"
#include <Rendering/Camera/Camera.h>
#include <Rendering/Graphics.h>
#include <iostream>
#include <cmath>
#include <Engine/Log.h>
#include <Engine/Subsystem/Console.h>
#include <GL/glew.h>
#include <Engine/EngineError.h>

float CSM::CSMDistance = 1;
const float CSM::cameraFarPlane = 250.f;
std::vector<float> CSM::shadowCascadeLevels{ cameraFarPlane / 9.f, cameraFarPlane / 3.f, cameraFarPlane / 1.0f };
unsigned int CSM::LightFBO = 0;
int CSM::Cascades = 3;
unsigned int CSM::ShadowMaps = 0;
unsigned int CSM::matricesUBO = 0;
Shader* CSM::ShadowShader = nullptr;

std::vector<glm::vec4> CSM::getFrustumCornersWorldSpace(const glm::mat4& projview)
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


void CSM::UpdateMatricesUBO(Camera* From)
{
	if (!Graphics::RenderShadows || Graphics::ShadowResolution <= 0)
	{
		return;
	}
	const auto LightSpaceMatrices = getLightSpaceMatrices(From);

	glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
	for (size_t i = 0; i < LightSpaceMatrices.size(); ++i)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &LightSpaceMatrices[i]);
	}
}

void CSM::BindLightSpaceMatricesToShader(const std::vector<glm::mat4>& Matrices, Shader* ShaderToBind)
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

std::vector<glm::vec4> CSM::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	return getFrustumCornersWorldSpace(proj * view);
}

CSM::CSM()
{
	Name = "Shadows";
	ShadowShader = new Shader("Shaders/Internal/shadow.vert", "Shaders/Internal/shadow.frag", "Shaders/Internal/shadow.geom");

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	}
	glGenBuffers(1, &matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 8, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	Console::ConsoleSystem->RegisterConVar(Console::Variable("shadow_resolution", NativeType::Int, &Graphics::ShadowResolution, CSM::ReInit));
	Console::ConsoleSystem->RegisterConVar(Console::Variable("shadows", NativeType::Bool, &Graphics::RenderShadows, CSM::ReInit));
	Console::ConsoleSystem->RegisterConVar(Console::Variable("shadow_distance", NativeType::Float, &CSMDistance, nullptr));
	ReInit();
}

void CSM::ReInit()
{
	if (ShadowMaps)
	{
		glDeleteTextures(1, &ShadowMaps);
		glDeleteFramebuffers(1, &LightFBO);
	}
	glGenFramebuffers(1, &LightFBO);
	ShadowMaps = 0;

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
		ENGINE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer should be complete.");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

glm::mat4 CSM::getLightSpaceMatrix(const float nearPlane, const float farPlane, Camera* From)
{
	const auto proj = glm::perspective(
		From->FOV, (float)Graphics::WindowResolution.X / (float)Graphics::WindowResolution.Y, nearPlane,
		farPlane);

	const auto corners = getFrustumCornersWorldSpace(proj, From->getView());

	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();
	float SnapSize = farPlane / 5;

	center = Vector3::SnapToGrid(center, SnapSize);
	auto lightView = glm::lookAt(center + (glm::vec3)Vector3::GetForwardVector(Graphics::WorldSun.Rotation), center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::min();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::min();
	for (const auto& v : corners)
	{
		const auto trf = lightView * v;
		minX = std::min(minX - SnapSize, trf.x);
		maxX = std::max(maxX + SnapSize, trf.x);
		minY = std::min(minY - SnapSize, trf.y);
		maxY = std::max(maxY + SnapSize, trf.y);
		minZ = std::min(minZ - SnapSize, trf.z);
		maxZ = std::max(maxZ + SnapSize, trf.z);
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

	float zoom = 2.0f;

	const glm::mat4 lightProjection = glm::ortho(minX / zoom, maxX / zoom, minY / zoom, maxY / zoom, minZ / zoom, maxZ / zoom);

	return lightProjection * lightView;
}

std::vector<glm::mat4> CSM::getLightSpaceMatrices(Camera* From)
{
	std::vector<glm::mat4> ret;
	for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			ret.push_back(getLightSpaceMatrix(0.1f * CSMDistance, shadowCascadeLevels[i] * CSMDistance, From));
		}
		else if (i < shadowCascadeLevels.size())
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1] * CSMDistance, shadowCascadeLevels[i] * CSMDistance, From));
		}
		else
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1] * CSMDistance, cameraFarPlane * CSMDistance, From));
		}
	}
	return ret;
}
#endif