#if !SERVER
#pragma once
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include <vector>
#include <Rendering/Renderable.h>

class Mesh
{
public:
	Mesh(std::vector<Vertex> Vertices, std::vector<unsigned int> Indices, Material Mat);
	~Mesh();

	void Render(Shader* UsedShader, bool MainFrameBuffer);
	void SimpleRender(Shader* UsedShader);

	void SetUniform(Material::Param NewUniform);

	ObjectRenderContext RenderContext;
	VertexBuffer* MeshVertexBuffer = nullptr;
protected:
private:
	int NumIndices;
	int NumVertices;
};
#endif