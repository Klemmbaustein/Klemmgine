#include "BillboardSprite.h"
#include <World/Graphics.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Rendering/VertexBuffer.h>
#include <Rendering/Shader.h>
#include <Engine/Log.h>
#include <GL/glew.h>

Shader* BillboardSprite::BillboardShader = nullptr;

BillboardSprite::BillboardSprite(unsigned int Texture, FramebufferObject* Buffer)
{
	if (!BillboardShader)
	{
		BillboardShader = new Shader("Shaders/Internal/billboard.vert", "Shaders/Internal/billboard.frag");
	}

	ParentBuffer = Buffer;
	Buffer->Renderables.push_back(this);
	TextureObject = Texture;
	BillboardVertexBuffer = new VertexBuffer(
		{
			Vertex(glm::vec3(-1, -1, 0), glm::vec2(0, 0)),
			Vertex(glm::vec3( 1, -1, 0), glm::vec2(1, 0)),
			Vertex(glm::vec3(-1,  1, 0), glm::vec2(0, 1)),
			Vertex(glm::vec3( 1,  1, 0), glm::vec2(1, 1)),
		},
		{
			0, 1, 2,
			1, 3, 2
		});
}

BillboardSprite::~BillboardSprite()
{
	for (FramebufferObject* o : Graphics::AllFramebuffers)
	{
		for (size_t i = 0; i < o->Renderables.size(); i++)
		{
			if (o->Renderables[i] == this)
			{
				o->Renderables.erase(o->Renderables.begin() + i);
				break;
			}
		}
	}
	delete BillboardVertexBuffer;
}

void BillboardSprite::Render(Camera* WorldCamera, bool MainFrameBuffer, bool TransparencyPass)
{
	if (!TransparencyPass)
	{
		return;
	}

	Vector3 rot = WorldCamera->Rotation;

	rot.Y = -rot.Y - 90;
	
	auto Mat = Transform(Position, rot.DegreesToRadiants(), ScaleWithDistance ? 1 : Vector3::Distance(WorldCamera->Position, Position)).ToMatrix();

	Mat = glm::rotate(Mat, glm::radians(Rotation), glm::vec3(0, 0, 1));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureObject);

	BillboardShader->Bind();
	BillboardShader->SetMat4("u_model", Mat);
	BillboardShader->SetFloat("u_scale", Size);
	BillboardShader->SetVector4("u_color", Vector4(Color, Opacity));
	BillboardShader->SetInt("u_texture", 0);
	BillboardShader->SetMat4("u_viewpro", WorldCamera->getViewProj());
	BillboardVertexBuffer->Draw();
	BillboardShader->Unbind();
}

void BillboardSprite::SimpleRender(Shader* UsedShader)
{
}