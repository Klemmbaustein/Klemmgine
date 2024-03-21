#if !SERVER
#include "Framebuffer.h"
#include <Rendering/Graphics.h>
#include <GL/glew.h>
#include <Engine/Log.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <Engine/Application.h>
#include <Rendering/Mesh/Model.h>
FramebufferObject::FramebufferObject()
{
	buf = new Framebuffer();
	buf->ReInit((int)(Graphics::WindowResolution.X), (int)(Graphics::WindowResolution.Y));
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