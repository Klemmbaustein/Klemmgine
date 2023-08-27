#include "Renderable.h"
#include <GL/glew.h>
#include <Rendering/Utility/CSM.h>
#include <Rendering/Graphics.h>
#include <Engine/Stats.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/Texture/Texture.h>
#include "Utility/ShaderManager.h"
#include <Rendering/Utility/BakedLighting.h>
#include <Engine/EngineError.h>

void Renderable::ApplyDefaultUniformsToShader(Shader* ShaderToApply)
{
	ShaderToApply->Bind();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, CSM::ShadowMaps);
	ShaderToApply->SetFloat("farPlane", CSM::cameraFarPlane);
	ShaderToApply->SetInt("cascadeCount", (int)CSM::shadowCascadeLevels.size());
	ShaderToApply->SetInt("shadowMap", 1);
	ShaderToApply->SetInt("GiMap", 3);
	ShaderToApply->SetInt("GiRes", (int)BakedLighting::GetLightTextureSize());
	ShaderToApply->SetVector3("GiScale", BakedLighting::GetLightMapScale());
	ShaderToApply->SetInt("Skybox", 2);
	ShaderToApply->SetFloat("u_biasmodifier", Vector3::Dot(
		Vector3::GetForwardVector(Graphics::MainCamera->Rotation),
		Graphics::WorldSun.Direction.Normalize()));
	ShaderToApply->SetVector3("u_cameraforward", Vector3::GetForwardVector(Graphics::MainCamera->Rotation));
	ShaderToApply->SetVector3("u_cameraposition", Graphics::MainFramebuffer->FramebufferCamera->Position);
	ShaderToApply->SetInt("u_shadowQuality", Graphics::PCFQuality);
	ShaderToApply->SetInt("u_shadows", Graphics::RenderShadows);
	ShaderToApply->SetFloat("FogFalloff", Graphics::WorldFog.Falloff);
	ShaderToApply->SetFloat("FogDistance", Graphics::WorldFog.Distance);
	ShaderToApply->SetFloat("FogMaxDensity", Graphics::WorldFog.MaxDensity);
	ShaderToApply->SetVector3("FogColor", Graphics::WorldFog.FogColor);

	ShaderToApply->SetVector3("u_directionallight.Direction", Graphics::WorldSun.Direction);
	ShaderToApply->SetFloat("u_directionallight.Intensity", Graphics::WorldSun.Intensity);
	ShaderToApply->SetFloat("u_directionallight.AmbientIntensity", Graphics::WorldSun.AmbientIntensity);
	ShaderToApply->SetVector3("u_directionallight.SunColor", Graphics::WorldSun.SunColor);
	ShaderToApply->SetVector3("u_directionallight.AmbientColor", Graphics::WorldSun.AmbientColor);

	ShaderToApply->SetFloat("u_time", Stats::Time);

	for (size_t i = 0; i < CSM::shadowCascadeLevels.size(); ++i)
	{
		glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(),
			((std::string("cascadePlaneDistances[") + std::to_string(i)) + "]").c_str()), CSM::shadowCascadeLevels[i] * CSM::CSMDistance);
	}
}

ObjectRenderContext::ObjectRenderContext(Material m)
{
	this->Mat = m;
	ContextShader = ReferenceShader("Shaders/" + m.VertexShader, "Shaders/" + m.FragmentShader);
	ENGINE_ASSERT(ContextShader, "Failed to load shader: " + m.VertexShader + ", " + m.FragmentShader);

	for (auto& i : m.Uniforms)
	{
		LoadUniform(i);
	}
}

ObjectRenderContext::ObjectRenderContext()
{
}

ObjectRenderContext::~ObjectRenderContext()
{

}

void ObjectRenderContext::Bind()
{
	BindWithShader(ContextShader);
}

void ObjectRenderContext::BindWithShader(Shader* s)
{
	s->Bind();
	GLint TexIterator = 0;
	for (int i = 0; i < Uniforms.size(); ++i)
	{
		if (!Uniforms.at(i).Content)
		{
			continue;
		}
		switch (Uniforms.at(i).Type)
		{
		case Type::Int:
			s->SetInt(Uniforms.at(i).Name, *static_cast<int*>(Uniforms.at(i).Content));
			break;
		case Type::Float:
			s->SetFloat(Uniforms.at(i).Name, *static_cast<float*>(Uniforms.at(i).Content));
			break;
		case Type::Vector3Color:
		case Type::Vector3:
			s->SetVector3(Uniforms.at(i).Name, *static_cast<Vector3*>(Uniforms.at(i).Content));
			break;
		case Type::GL_Texture:
			glActiveTexture(GL_TEXTURE7 + TexIterator);
			glBindTexture(GL_TEXTURE_2D, *(unsigned int*)Uniforms.at(i).Content);
			glUniform1i(glGetUniformLocation(s->GetShaderID(), Uniforms.at(i).Name.c_str()), 7 + TexIterator);
			TexIterator++;
			break;
		default:
			break;
		}
	}
}

Shader* ObjectRenderContext::GetShader()
{
	return ContextShader;
}

void ObjectRenderContext::LoadUniform(Material::Param u)
{
	size_t UniformIndex = SIZE_MAX;
	for (size_t i = 0; i < Uniforms.size(); i++)
	{
		if (Uniforms[i].Type == u.Type && Uniforms[i].Name == u.UniformName)
		{
			UniformIndex = i;
			return;
		}
	}

	if (UniformIndex == SIZE_MAX)
	{
		Uniforms.push_back(Uniform(u.UniformName, u.Type, nullptr));
		UniformIndex = Uniforms.size() - 1;
	}

	if (u.Value.empty() && u.Type != Type::Bool)
	{
		return;
	}
	try
	{
		switch (u.Type)
		{
		case Type::Int:
			Uniforms[UniformIndex].Content = new int(std::stoi(u.Value));
			break;
		case Type::Float:
			Uniforms[UniformIndex].Content = new float(std::stof(u.Value));
			break;
		case Type::Vector3Color:
		case Type::Vector3:
			Uniforms[UniformIndex].Content = new Vector3(Vector3::stov(u.Value));
			break;
		case Type::GL_Texture:
			Uniforms[UniformIndex].Content = (void*)new unsigned int(Texture::LoadTexture(u.Value));
			break;
		default:
			break;
		}
	}
	catch (std::exception)
	{
		switch (u.Type)
		{
		case Type::Int:
			Uniforms[UniformIndex].Content = new int(0);
			break;
		case Type::Float:
			Uniforms[UniformIndex].Content = new float(0);
			break;
		case Type::Vector3Color:
		case Type::Vector3:
			Uniforms[UniformIndex].Content = new Vector3();
			break;
		case Type::GL_Texture:
			Uniforms[UniformIndex].Content = new unsigned int(0);
			break;
		default:
			break;
		}
	}
}

void ObjectRenderContext::Unload()
{
	for (Uniform& u : Uniforms)
	{
		if (!u.Content)
		{
			continue;
		}
		switch (u.Type)
		{
		case Type::Int:
			delete reinterpret_cast<int*>(u.Content);
			break;
		case Type::Float:
			delete reinterpret_cast<float*>(u.Content);
			break;
		case Type::Vector3:
		case Type::Vector3Color:
			delete reinterpret_cast<Vector3*>(u.Content);
			break;
		case Type::GL_Texture:
			delete reinterpret_cast<unsigned int*>(u.Content);
			break;
		default:
			break;
		}
	}
	Uniforms.clear();
	DereferenceShader("Shaders/" + Mat.VertexShader, "Shaders/" + Mat.FragmentShader);
	ContextShader = nullptr;
	Mat = Material();
}
