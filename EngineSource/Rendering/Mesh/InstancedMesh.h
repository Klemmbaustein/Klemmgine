#pragma once
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include <vector>
#include "Rendering/Utility/IndexBuffer.h"
#include <Rendering/Mesh/Mesh.h>

class InstancedMesh
{
public:
	InstancedMesh(std::vector<Vertex> Vertices, std::vector<int> Indices, Material Mat);
	~InstancedMesh();

	void Render(Shader* UsedShader);
	void SimpleRender(Shader* UsedShader);

	void SetUniform(Material::Param NewUniform);

	void SetInstances(std::vector<Transform> T);
	VertexBuffer* MeshVertexBuffer = nullptr;
	ObjectRenderContext RenderContext;

protected:
private:
	std::vector<Transform> Instances;
	IndexBuffer* MeshIndexBuffer;
	int NumIndices;
	int NumVertices;
};

