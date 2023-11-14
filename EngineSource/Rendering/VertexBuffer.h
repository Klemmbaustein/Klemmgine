#pragma once
#include <vector>
#include "Vertex.h"
#if !SERVER
struct VertexBuffer
{
	unsigned int VAO = 0u, VBO = 0u, EBO = 0u, IndicesSize = 0u;
	std::vector<Vertex> Vertices; std::vector<unsigned int> Indices;
public:
	static VertexBuffer* MakeSquare();

	VertexBuffer(std::vector<Vertex> Vertices, std::vector<unsigned int> Indices);
	~VertexBuffer();
	void Bind();
	void Unbind();

	void Draw();
};
#endif