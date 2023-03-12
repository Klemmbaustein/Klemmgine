#pragma once

#include "Rendering/Vertex.h"

struct IndexBuffer
{
	IndexBuffer(void* data, int NumVertecies, int ElementSize);

	virtual ~IndexBuffer();

	void Bind();

	void Unbind();


private:
	unsigned int BufferID;
};