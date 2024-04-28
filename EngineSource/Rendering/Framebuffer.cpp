#if !SERVER
#include "Framebuffer.h"
#include <Rendering/Graphics.h>
#include <GL/glew.h>
#include <Engine/Log.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <Engine/Application.h>
#include <Rendering/Mesh/Model.h>
#include <Engine/Stats.h>
#include "RenderSubsystem/CSM.h"
#include "ShaderManager.h"
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Rendering/Texture/Cubemap.h>
#include <Rendering/RenderSubsystem/OcclusionCulling.h>
#include <Engine/EngineError.h>

FramebufferObject::FramebufferObject()
{
	buf = new Framebuffer();
	buf->ReInit((int)(Graphics::RenderResolution.X), (int)(Graphics::RenderResolution.Y));
	Graphics::AllFramebuffers.push_back(this);
}

FramebufferObject::~FramebufferObject()
{
	for (unsigned int i = 0; i < Graphics::AllFramebuffers.size(); i++)
	{
		if (Graphics::AllFramebuffers[i] == this)
		{
			Graphics::AllFramebuffers.erase(Graphics::AllFramebuffers.begin() + i);
		}
	}
	delete buf;
}

unsigned int FramebufferObject::GetTextureID()
{
	return buf->GetTextureID(0);
}

void FramebufferObject::ReInit()
{
	if (!UseMainWindowResolution)
	{
		buf->ReInit((unsigned int)(CustomFramebufferResolution.X),
			(unsigned int)(CustomFramebufferResolution.Y));

		FramebufferCamera->ReInit(FramebufferCamera->FOV,
			CustomFramebufferResolution.X,
			CustomFramebufferResolution.Y);
	}
	else
	{
		buf->ReInit((unsigned int)(Graphics::RenderResolution.X),
			(unsigned int)(Graphics::RenderResolution.Y));

		FramebufferCamera->ReInit(FramebufferCamera->FOV, Graphics::RenderResolution.X, Graphics::RenderResolution.X);
	}
}

void FramebufferObject::UseWith(Renderable* r)
{
	Renderables.push_back(r);
}

Framebuffer* FramebufferObject::GetBuffer()
{
	return buf;
}

void FramebufferObject::Draw()
{
#if !SERVER
	if (!FramebufferCamera) return;

#if EDITOR

	if (!Application::WindowHasFocus())
	{
		return;
	}

#endif

	FramebufferCamera->Update();

	// If there's nothing to render, just clear it.8516
	if ((!Renderables.size() && !ParticleEmitters.size()) || !Active)
	{
		GetBuffer()->Bind();
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		return;
	}

	if (PreviousReflectionCubemapName != ReflectionCubemapName)
	{
		PreviousReflectionCubemapName = ReflectionCubemapName;
		if (ReflectionCubemap)
		{
			Cubemap::UnloadCubemapFile(ReflectionCubemap);
		}
		if (!ReflectionCubemapName.empty())
		{
			ReflectionCubemap = Cubemap::LoadCubemapFile(ReflectionCubemapName);
		}
		else
		{
			ReflectionCubemap = 0;
		}
	}

	for (auto* p : ParticleEmitters)
	{
		p->Update(FramebufferCamera);
	}
	Stats::EngineStatus = "Rendering (Framebuffer: Shadows)";
	FrustumCulling::Active = false;
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.f, 0.f, 0.f, 1.f);		//Clear color black
	glViewport(0, 0, Graphics::ShadowResolution, Graphics::ShadowResolution);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CSM::UpdateMatricesUBO(FramebufferCamera);

	if (Graphics::RenderShadows && Graphics::ShadowResolution > 0 && !Graphics::RenderFullbright)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, CSM::LightFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		for (int j = 0; j < Renderables.size(); j++)
		{
			if (Renderables[j]->CastShadow)
				Renderables.at(j)->SimpleRender(CSM::ShadowShader);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	FrustumCulling::Active = false;
	FrustumCulling::CurrentCameraFrustum = FrustumCulling::createFrustumFromCamera(*FramebufferCamera);
	GetBuffer()->Bind();
	Stats::EngineStatus = "Rendering (Framebuffer: Main pass)";

	Vector2 BufferResolution = UseMainWindowResolution ? Graphics::RenderResolution : CustomFramebufferResolution;
	glViewport(0, 0, (int)BufferResolution.X, (int)BufferResolution.Y);
	const auto LightSpaceMatrices = CSM::getLightSpaceMatrices(FramebufferCamera);

	std::vector<Graphics::Light*> DrawnLights;
	DrawnLights.reserve(std::min(size_t(8), Lights.size()));

	for (auto& Light : Lights)
	{
		float Distance = Vector3::DistanceSquared(FramebufferCamera->Position, Light.Position);
		if (DrawnLights.size() < Graphics::MAX_LIGHTS)
		{
			Light.Distance = Distance;
			DrawnLights.push_back(&Light);
		}
		else
		{
			for (Graphics::Light*& i : DrawnLights)
			{
				if (Distance < i->Distance)
				{
					i = &Light;
					i->Distance = Distance;
					break;
				}
			}
		}
	}

	for (auto& s : ShaderManager::Shaders)
	{
		Renderable::ApplyDefaultUniformsToShader(s.second.UsedShader, this == Graphics::MainFramebuffer);
		CSM::BindLightSpaceMatricesToShader(LightSpaceMatrices, s.second.UsedShader);

		for (int i = 0; i < Graphics::MAX_LIGHTS; i++)
		{
			std::string CurrentLight = "u_lights[" + std::to_string(i) + "]";
			if (i < DrawnLights.size())
			{
				s.second.UsedShader->SetVector3(CurrentLight + ".Position", DrawnLights[i]->Position);
				s.second.UsedShader->SetVector3(CurrentLight + ".Color", DrawnLights[i]->Color);
				s.second.UsedShader->SetFloat(CurrentLight + ".Falloff", DrawnLights[i]->Falloff);
				s.second.UsedShader->SetFloat(CurrentLight + ".Intensity", DrawnLights[i]->Intensity);

				s.second.UsedShader->SetInt(CurrentLight + ".Active", 1);
			}
			else
			{
				s.second.UsedShader->SetInt(CurrentLight + ".Active", 0);
			}
		}
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (Graphics::IsWireframe)
	{
		glLineWidth(3);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	// Bind cubemap texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ReflectionCubemap);
	// Main pass

	OcclusionCulling* OcclusionManager = static_cast<OcclusionCulling*>(Subsystem::GetSubsystemByName("Occlude"));

	ENGINE_ASSERT(OcclusionManager, "Occlusion manager must be loaded");

	BakedLighting::BindToTexture();

	OcclusionManager->CullShader->Bind();
	OcclusionManager->CullShader->SetMat4("u_viewpro", FramebufferCamera->getViewProj());
	size_t i = 0;
	GetBuffer()->Bind();
	for (auto o : Renderables)
	{
		Model* m = dynamic_cast<Model*>(o);
		if (this == Graphics::MainFramebuffer && m && !Graphics::IsWireframe)
		{
			OcclusionManager->RenderOccluded(m, i, this);
		}
		else
		{
			o->Render(FramebufferCamera, this == Graphics::MainFramebuffer, false);
		}
		i++;
	}
	for (size_t i = 0; i < Renderables.size(); i++)
	{
		Model* m = dynamic_cast<Model*>(Renderables[i]);
		if (!m)
		{
			continue;
		}
		OcclusionManager->UpdateOcclusionStatus(m);
	}

	OcclusionManager->CheckQueries(this);

	GetBuffer()->Bind();
	// Transparency pass
	for (auto p : ParticleEmitters)
	{
		p->Draw(FramebufferCamera, this == Graphics::MainFramebuffer, true);
	}
	GetBuffer()->Bind();
	for (auto o : Renderables)
	{
		o->Render(FramebufferCamera, this == Graphics::MainFramebuffer, true);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, (int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
#endif
}
void FramebufferObject::AddEditorGrid()
{
	ModelGenerator::ModelData PlaneMesh;
	auto& Elem = PlaneMesh.AddElement();
	Elem.AddFace(2, Vector3(0, 1, 0), Vector3(0, -0.5f, 0));
	Elem.ElemMaterial = Application::GetEditorPath() + "/EditorContent/Materials/WorldGrid.jsmat";
	PlaneMesh.CastShadow = false;
	PlaneMesh.TwoSided = true;
	Model* GridModel = new Model(PlaneMesh);
	GridModel->ModelTransform.Scale = 10000;
	GridModel->UpdateTransform();
	GridModel->DestroyOnUnload = false;
	Renderables.push_back(GridModel);
}

void FramebufferObject::ClearContent(bool Full)
{
	std::vector<Renderable*> Remaining;
	for (Renderable* r : Renderables)
	{
		if (r->DestroyOnUnload || Full)
		{
			delete r;
		}
		else
		{
			Remaining.push_back(r);
		}
	}
	Lights.clear();
	Renderables = Remaining;
	ParticleEmitters.clear();
}

void Framebuffer::AttachFramebuffer(unsigned int Buffer, unsigned int Attachement)
{
	Bind();
	GLuint* NewArray = new GLuint[numbuffers + 1];
	for (unsigned int i = 0; i < numbuffers; i++)
	{
		NewArray[i] = Textures[i];
	}
	NewArray[numbuffers] = Buffer;
	numbuffers++;
	Textures = NewArray;
	Attachements.push_back(Attachement);
	Unbind();
}

Framebuffer::Framebuffer()
{
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &FBO);
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::GetTextureID(int Index)
{
	return Textures[Index];
}

void Framebuffer::ReInit(int Width, int Height, bool ColorAttachementType)
{
	if (FBO)
	{
		glDeleteFramebuffers(1, &FBO);
		glDeleteTextures(numbuffers, Textures);
	}

	glGenFramebuffers(1, &FBO);
	glGenTextures(numbuffers, Textures);
	for (unsigned int i = 0; i < numbuffers; i++)
	{
		Vector4 BorderColor = (Attachements[i] == GL_DEPTH_STENCIL_ATTACHMENT) ? Vector4(1) : Vector4(0, 0, 0, 1);

		glBindTexture(GL_TEXTURE_2D, Textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0,
			(Attachements[i] == GL_DEPTH_STENCIL_ATTACHMENT) ? GL_DEPTH24_STENCIL8 : GL_RGBA16F, Width, Height, 0, (Attachements[i] == GL_DEPTH_STENCIL_ATTACHMENT) ? GL_DEPTH_STENCIL : GL_RGBA,
			Attachements[i] == GL_DEPTH_STENCIL_ATTACHMENT ? GL_UNSIGNED_INT_24_8 : GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Graphics::RenderAntiAlias ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Graphics::RenderAntiAlias ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &BorderColor.X);
	}

	Bind();
	for (unsigned int i = 0; i < numbuffers; i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachements[i], GL_TEXTURE_2D, Textures[i], 0);
	}
	Unbind();
}
#endif