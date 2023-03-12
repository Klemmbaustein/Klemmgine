#include "IndexBuffer.h"
#include <GL/glew.h>

IndexBuffer::IndexBuffer(void* data, int NumVertecies, int ElementSize)
{
	glGenBuffers(1, &BufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumVertecies * ElementSize, data, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &BufferID);
}

void IndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferID);
}

void IndexBuffer::Unbind()
{
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
