#if !SERVER
#include "InstancedMesh.h"
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include <vector>
#include <fstream>
#include <Rendering/Texture/Texture.h>
#include <GL/glew.h>

InstancedMesh::InstancedMesh(std::vector<Vertex> Vertices, std::vector<unsigned int> Indices, Material Mat)
{
	NumVertices = (int)Vertices.size();
	NumIndices = (int)Indices.size();
	MeshVertexBuffer = new VertexBuffer(Vertices, Indices);
	RenderContext = ObjectRenderContext(Mat);
}


InstancedMesh::~InstancedMesh()
{
	delete MeshVertexBuffer;
	RenderContext.Unload();
}
void InstancedMesh::Render(Shader* UsedShader, bool MainFramebuffer)
{
	MeshVertexBuffer->Bind();
	RenderContext.Bind();
	if (MainFramebuffer)
	{
		unsigned int attachements[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachements);
	}
	else
	{
		unsigned int attachements[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachements);
	}
	glDrawElementsInstanced(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0, (int)Instances.size());
	MeshVertexBuffer->Unbind();
}

void InstancedMesh::SimpleRender(Shader* UsedShader)
{
	if (RenderContext.Mat.UseShadowCutout)
	{
		RenderContext.BindWithShader(UsedShader);
		UsedShader->SetInt("u_useTexture", 1);
	}
	else
	{
		UsedShader->SetInt("u_useTexture", 0);
	}
	MeshVertexBuffer->Bind();
	glDrawElementsInstanced(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0, (int)Instances.size());
	MeshVertexBuffer->Unbind();
}

void InstancedMesh::SetUniform(Material::Param NewUniform)
{
	RenderContext.LoadUniform(NewUniform);
}

void InstancedMesh::SetInstances(std::vector<Transform> T)
{
	Instances = T;
}
#endif