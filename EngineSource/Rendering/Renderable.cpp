#include "Renderable.h"
#include <GL/glew.h>
#include <Rendering/RenderSubsystem/CSM.h>
#include <Rendering/Graphics.h>
#include <Engine/Stats.h>
#include <Rendering/Framebuffer.h>
#include <Rendering/Texture/Texture.h>
#include "ShaderManager.h"
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Engine/EngineError.h>
#include <Engine/Log.h>
#include "ShaderPreprocessor.h"

Graphics::Sun FullbrightSun =
{
	.Intensity = 0,
	.AmbientIntensity = 1,
	.SunColor = 1,
	.AmbientColor = 1,
};

void Renderable::ApplyDefaultUniformsToShader(Shader* ShaderToApply, bool MainFramebuffer)
{
#if !SERVER
	ShaderToApply->Bind();
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, CSM::ShadowMaps);
	ShaderToApply->SetFloat("farPlane", CSM::cameraFarPlane);
	ShaderToApply->SetInt("cascadeCount", (int)CSM::shadowCascadeLevels.size());
	ShaderToApply->SetInt("shadowMap", 1);
	ShaderToApply->SetInt("GiMap", 3);
	ShaderToApply->SetInt("GiRes", (int)BakedLighting::GetLightTextureSize());
	ShaderToApply->SetInt("GiEnabled", (int)(BakedLighting::LoadedLightmap && MainFramebuffer));
	ShaderToApply->SetVector3("GiScale", BakedLighting::GetLightMapScale());
	ShaderToApply->SetInt("Skybox", 2);
	ShaderToApply->SetFloat("u_biasmodifier", Vector3::Dot(
		Vector3::GetForwardVector(Graphics::MainCamera->Rotation),
		Vector3::GetForwardVector(Graphics::WorldSun.Rotation)));
	ShaderToApply->SetVector3("u_cameraforward", Vector3::GetForwardVector(Graphics::MainCamera->Rotation));
	ShaderToApply->SetVector3("u_cameraposition", Graphics::MainFramebuffer->FramebufferCamera->Position);
	ShaderToApply->SetInt("u_shadowQuality", Graphics::PCFQuality);
	ShaderToApply->SetInt("u_shadows", Graphics::RenderShadows);
	ShaderToApply->SetFloat("FogFalloff", Graphics::WorldFog.Falloff);
	ShaderToApply->SetFloat("FogDistance", Graphics::WorldFog.Distance);
	ShaderToApply->SetFloat("FogMaxDensity", Graphics::WorldFog.MaxDensity);
	ShaderToApply->SetVector3("FogColor", Graphics::WorldFog.FogColor);

	ShaderToApply->SetVector3("u_directionallight.Direction", Vector3::GetForwardVector(Graphics::WorldSun.Rotation));

	const Graphics::Sun& UsedSun = Graphics::RenderFullBright ? FullbrightSun : Graphics::WorldSun;

	ShaderToApply->SetFloat("u_directionallight.Intensity", UsedSun.Intensity);
	ShaderToApply->SetFloat("u_directionallight.AmbientIntensity", UsedSun.AmbientIntensity);
	ShaderToApply->SetVector3("u_directionallight.SunColor", UsedSun.SunColor);
	ShaderToApply->SetVector3("u_directionallight.AmbientColor", UsedSun.AmbientColor);

	ShaderToApply->SetFloat("u_time", Stats::Time);

	for (size_t i = 0; i < CSM::shadowCascadeLevels.size(); ++i)
	{
		glUniform1f(glGetUniformLocation(ShaderToApply->GetShaderID(),
			((std::string("cascadePlaneDistances[") + std::to_string(i)) + "]").c_str()), CSM::shadowCascadeLevels[i] * CSM::CSMDistance);
	}
#endif
}

ObjectRenderContext::ObjectRenderContext(Material m)
{
#if !SERVER
	this->Mat = m;
	ContextShader = ShaderManager::ReferenceShader("Shaders/" + m.VertexShader, "Shaders/" + m.FragmentShader);
	if (!ContextShader)
	{
#if RELEASE
		ENGINE_ASSERT(ContextShader, "Failed to load shader - " + m.VertexShader + ", " + m.FragmentShader);
#endif
		ContextShader = ShaderManager::ReferenceShader("Shaders/basic.vert", "Shaders/basic.frag");

		m.Uniforms = Preprocessor::ParseGLSL(FileUtil::GetFileContent("Shaders/basic.frag"), "Shaders/").ShaderParams;
	}
	for (auto& i : m.Uniforms)
	{
		LoadUniform(i);
	}
#endif
}

ObjectRenderContext::ObjectRenderContext()
{
}

ObjectRenderContext::~ObjectRenderContext()
{
}

void ObjectRenderContext::Bind()
{
#if !SERVER
	BindWithShader(ContextShader);
#endif
}

void ObjectRenderContext::BindWithShader(Shader* s)
{
#if !SERVER
	s->Bind();
	GLint TexIterator = 0;
	for (int i = 0; i < Uniforms.size(); ++i)
	{
		if (!Uniforms.at(i).Content)
		{
			continue;
		}
		switch (Uniforms.at(i).NativeType)
		{
		case NativeType::Bool:
		case NativeType::Int:
			s->SetInt(Uniforms.at(i).Name, *static_cast<int*>(Uniforms.at(i).Content));
			break;
		case NativeType::Float:
			s->SetFloat(Uniforms.at(i).Name, *static_cast<float*>(Uniforms.at(i).Content));
			break;
		case NativeType::Vector3Color:
		case NativeType::Vector3:
			s->SetVector3(Uniforms.at(i).Name, *static_cast<Vector3*>(Uniforms.at(i).Content));
			break;
		case NativeType::GL_Texture:
			glActiveTexture(GL_TEXTURE7 + TexIterator);
			glBindTexture(GL_TEXTURE_2D, *(unsigned int*)Uniforms.at(i).Content);
			glUniform1i(glGetUniformLocation(s->GetShaderID(), Uniforms.at(i).Name.c_str()), 7 + TexIterator);
			TexIterator++;
			break;
		default:
			break;
		}
	}
#endif
}

Shader* ObjectRenderContext::GetShader()
{
#if !SERVER
	return ContextShader;
#endif
	return nullptr;
}

void ObjectRenderContext::LoadUniform(Material::Param u)
{
#if !SERVER
	size_t UniformIndex = SIZE_MAX;
	for (size_t i = 0; i < Uniforms.size(); i++)
	{
		if (Uniforms[i].NativeType == u.NativeType && Uniforms[i].Name == u.UniformName)
		{
			UniformIndex = i;
			switch (u.NativeType)
			{
			case NativeType::Int:
			case NativeType::Bool:
				delete (int*)Uniforms[i].Content;
				break;
			case NativeType::Vector3Color:
			case NativeType::Vector3Rotation:
			case NativeType::Vector3:
				delete (Vector3*)Uniforms[i].Content;
				break;
			case NativeType::Float:
				delete (float*)Uniforms[i].Content;
				break;
			case NativeType::GL_Texture:
				delete (unsigned int*)Uniforms[i].Content;
				break;
			default:
				break;
			}
			return;
		}
	}

	if (UniformIndex == SIZE_MAX)
	{
		Uniforms.push_back(Uniform(u.UniformName, u.NativeType, nullptr));
		UniformIndex = Uniforms.size() - 1;
	}

	try
	{
		switch (u.NativeType)
		{
		case NativeType::Int:
		case NativeType::Bool:
			Uniforms[UniformIndex].Content = new int(std::stoi(u.Value));
			break;
		case NativeType::Float:
			Uniforms[UniformIndex].Content = new float(std::stof(u.Value));
			break;
		case NativeType::Vector3Color:
		case NativeType::Vector3Rotation:
		case NativeType::Vector3:
			Uniforms[UniformIndex].Content = new Vector3(Vector3::FromString(u.Value));
			break;
		case NativeType::GL_Texture:
		{
			Texture::TextureInfo TextureInfo = Texture::ParseTextureInfoString(u.Value);
			unsigned int LoadedTexture = Texture::LoadTexture(TextureInfo);
			if (LoadedTexture)
			{
				Uniforms[UniformIndex].Content = (void*)new unsigned int(LoadedTexture);
			}
			else
			{
				Uniforms[UniformIndex].Content = (void*)new unsigned int(0);
			}
			break;
		}
		default:
			break;
		}
	}
	catch (std::exception& e)
	{
		Log::Print(e.what(), Log::LogColor::Yellow);
		switch (u.NativeType)
		{
		case NativeType::Int:
		case NativeType::Bool:
			Uniforms[UniformIndex].Content = new int(0);
			break;
		case NativeType::Float:
			Uniforms[UniformIndex].Content = new float(0);
			break;
		case NativeType::Vector3Color:
		case NativeType::Vector3:
			Uniforms[UniformIndex].Content = new Vector3();
			break;
		case NativeType::GL_Texture:
			Uniforms[UniformIndex].Content = new unsigned int(0);
			break;
		default:
			break;
		}
	}
#endif
}

void ObjectRenderContext::Unload()
{
#if !SERVER
	for (Uniform& u : Uniforms)
	{
		if (!u.Content)
		{
			continue;
		}
		switch (u.NativeType)
		{
		case NativeType::Int:
		case NativeType::Bool:
			delete reinterpret_cast<int*>(u.Content);
			break;
		case NativeType::Float:
			delete reinterpret_cast<float*>(u.Content);
			break;
		case NativeType::Vector3:
		case NativeType::Vector3Color:
			delete reinterpret_cast<Vector3*>(u.Content);
			break;
		case NativeType::GL_Texture:
			delete reinterpret_cast<unsigned int*>(u.Content);
			break;
		default:
			break;
		}
	}
	Uniforms.clear();
	ShaderManager::DereferenceShader("Shaders/" + Mat.VertexShader, "Shaders/" + Mat.FragmentShader);
	ContextShader = nullptr;
	Mat = Material();
#endif
}
