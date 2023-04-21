#include "InstancedMesh.h"
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include "Rendering/Utility/IndexBuffer.h"
#include <vector>
#include <fstream>
#include <Rendering/Texture/Texture.h>
#include <GL/glew.h>

InstancedMesh::InstancedMesh(std::vector<Vertex> Vertices, std::vector<int> Indices, Material Mat)
{
	NumVertices = Vertices.size();
	NumIndices = Indices.size();
	MeshIndexBuffer = new IndexBuffer(Indices.data(), NumIndices, sizeof(Indices[0]));
	MeshVertexBuffer = new VertexBuffer(Vertices.data(), NumVertices);
	RenderContext = ObjectRenderContext(Mat);
}


InstancedMesh::~InstancedMesh()
{
	delete MeshVertexBuffer;
	delete MeshIndexBuffer;
	RenderContext.Unload();
}
void InstancedMesh::Render(Shader* UsedShader)
{
	RenderContext.Bind();
	MeshVertexBuffer->Bind();
	MeshIndexBuffer->Bind();
	unsigned int attachements[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachements);
	glDrawElementsInstanced(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0, Instances.size());
	MeshVertexBuffer->Unbind();
	MeshIndexBuffer->Unbind();
}
void InstancedMesh::SimpleRender(Shader* UsedShader)
{
	if (MeshMaterial.UseShadowCutout)
	{
		RenderContext.Bind();
	}
	else
	{
		RenderContext.GetShader()->SetInt("u_usetexture", 0);
	}
	MeshVertexBuffer->Bind();
	MeshIndexBuffer->Bind();
	glDrawElementsInstanced(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0, Instances.size());
	MeshVertexBuffer->Unbind();
	MeshIndexBuffer->Unbind();
}

void InstancedMesh::SetUniform(Material::Param NewUniform)
{
	RenderContext.LoadUniform(NewUniform);
}

void InstancedMesh::SetInstances(std::vector<Transform> T)
{
	Instances = T;
}