#include "VertexBuffer.h"
#include <GL/glew.h>

VertexBuffer::VertexBuffer(void* data, int NumVertices)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &BufferID);
	glBindBuffer(GL_ARRAY_BUFFER, BufferID);
	glBufferData(GL_ARRAY_BUFFER, NumVertices * sizeof(Vertex), data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(struct Vertex, Position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(struct Vertex, U));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(struct Vertex, ColorR));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(struct Vertex, Normal));

	glBindVertexArray(0);
}

unsigned int VertexBuffer::GetVAO()
{
	return VAO;
}

void VertexBuffer::SetData(void* data, int NumVertices)
{
	glBufferData(GL_ARRAY_BUFFER, NumVertices * sizeof(Vertex), data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &BufferID);
	glDeleteVertexArrays(1, &VAO);
}

void VertexBuffer::Bind()
{
	glBindVertexArray(VAO);
}

void VertexBuffer::Unbind()
{
	glBindVertexArray(0);
}