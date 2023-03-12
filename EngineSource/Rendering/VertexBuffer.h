#pragma once

#include <iostream>
#include "Vertex.h"

struct VertexBuffer
{
	VertexBuffer(void* data, int NumVertices);

	VertexBuffer() {}

	unsigned int GetVAO();

	void SetData(void* data, int NumVertices);

	virtual ~VertexBuffer();

	void Bind();

	void Unbind();

	unsigned int BufferID = 0;
private:
	unsigned int VAO = 0;
};