#include "Mesh.h"
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include <vector>
#include <fstream>
#include <Rendering/Texture/Texture.h>
#include <GL/glew.h>
#include <Engine/Log.h>
#if !SERVER
Mesh::Mesh(std::vector<Vertex> Vertices, std::vector<unsigned int> Indices, Material Mat)
{
	NumVertices = (int)Vertices.size();
	NumIndices = (int)Indices.size();
	MeshVertexBuffer = new VertexBuffer(Vertices, Indices);
	RenderContext = ObjectRenderContext(Mat);
}


Mesh::~Mesh()
{
	delete MeshVertexBuffer;
	RenderContext.Unload();
}
void Mesh::Render(Shader* UsedShader, bool MainFrameBuffer)
{
	RenderContext.Bind();
	MeshVertexBuffer->Bind();
	if (MainFrameBuffer)
	{
		unsigned int attachements[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachements);
	}
	else
	{
		unsigned int attachements[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachements);
	}
	MeshVertexBuffer->Draw();
}
void Mesh::SimpleRender(Shader* UsedShader)
{
	UsedShader->Bind();
	if (RenderContext.Mat.UseShadowCutout)
	{
		RenderContext.BindWithShader(UsedShader);
		UsedShader->SetInt("u_useTexture", 1);
	}
	else
	{
		UsedShader->SetInt("u_useTexture", 0);
	}
	MeshVertexBuffer->Draw();
}

void Mesh::SetUniform(Material::Param NewUniform)
{
	RenderContext.LoadUniform(NewUniform);
}
#endif