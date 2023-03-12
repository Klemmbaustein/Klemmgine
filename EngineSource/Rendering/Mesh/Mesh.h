#pragma once
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include <vector>
#include "Rendering/Utility/IndexBuffer.h"
#include <Rendering/Texture/Material.h>

struct Uniform
{
	std::string Name;
	int Type;
	void* Content;
	Uniform(std::string Name, int Type, void* Content)
	{
		this->Content = Content;
		this->Name = Name;
		this->Type = Type;
	}
};


class Mesh
{
public:
	Mesh(std::vector<Vertex> Vertices, std::vector<int> Indices);

	void Render(Shader* UsedShader, bool MainFrameBuffer);


	void SimpleRender(Shader* UsedShader);

	~Mesh();

	void ApplyUniforms();
	void ApplyUniform(size_t Index);
	std::vector<Uniform> Uniforms;
	Shader* MeshShader = nullptr;
	VertexBuffer* MeshVertexBuffer = nullptr;
	Material MeshMaterial;
protected:
private:
	IndexBuffer* MeshIndexBuffer;
	int NumIndices;
	int NumVertices;
};

