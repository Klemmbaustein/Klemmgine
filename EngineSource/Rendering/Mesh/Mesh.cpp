#include "Mesh.h"
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include "Rendering/Utility/IndexBuffer.h"
#include <vector>
#include <fstream>
#include <Rendering/Texture/Texture.h>
#include <GL/glew.h>
#include <Engine/Log.h>

Mesh::Mesh(std::vector<Vertex> Vertices, std::vector<int> Indices, Material Mat)
{
	NumVertices = Vertices.size();
	NumIndices = Indices.size();
	MeshIndexBuffer = new IndexBuffer(Indices.data(), NumIndices, sizeof(Indices[0]));
	MeshVertexBuffer = new VertexBuffer(Vertices.data(), NumVertices);
	RenderContext = ObjectRenderContext(Mat);
}


Mesh::~Mesh()
{
	delete MeshVertexBuffer;
	delete MeshIndexBuffer;
	RenderContext.Unload();
}
void Mesh::Render(Shader* UsedShader, bool MainFrameBuffer)
{
	MeshVertexBuffer->Bind();
	MeshIndexBuffer->Bind();
	RenderContext.Bind();
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
	glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);
	MeshVertexBuffer->Unbind();
	MeshIndexBuffer->Unbind();
}
void Mesh::SimpleRender(Shader* UsedShader)
{
	UsedShader->Bind();
	if (RenderContext.Mat.UseShadowCutout)
	{
		RenderContext.BindWithShader(UsedShader);
		UsedShader->SetInt("u_usetexture", 1);
	}
	else
	{
		UsedShader->SetInt("u_usetexture", 0);
	}
	MeshVertexBuffer->Bind();
	MeshIndexBuffer->Bind();
	glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);
	MeshVertexBuffer->Unbind();
	MeshIndexBuffer->Unbind();
}

void Mesh::SetUniform(Material::Param NewUniform)
{
	RenderContext.LoadUniform(NewUniform);
}
