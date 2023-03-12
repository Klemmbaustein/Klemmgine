#include "Mesh.h"
#include "Math/Vector.h"
#include "Rendering/Shader.h"
#include "Rendering/VertexBuffer.h"
#include "Rendering/Utility/IndexBuffer.h"
#include <vector>
#include <fstream>
#include <Rendering/Texture/Texture.h>
#include <GL/glew.h>

Mesh::Mesh(std::vector<Vertex> Vertices, std::vector<int> Indices)
{
	NumVertices = Vertices.size();
	NumIndices = Indices.size();
	MeshIndexBuffer = new IndexBuffer(Indices.data(), NumIndices, sizeof(Indices[0]));
	MeshVertexBuffer = new VertexBuffer(Vertices.data(), NumVertices);

}


Mesh::~Mesh()
{
	delete MeshVertexBuffer;
	delete MeshIndexBuffer;
	for (Uniform& u : Uniforms)
	{
		switch (u.Type)
		{
		case Type::E_INT:
			delete reinterpret_cast<int*>(u.Content);
			break;
		case Type::E_FLOAT:
			delete reinterpret_cast<float*>(u.Content);
			break;
		case Type::E_VECTOR3:
		case Type::E_VECTOR3_COLOR:
			delete reinterpret_cast<Vector3*>(u.Content);
			break;
		case Type::E_GL_TEXTURE:
			delete reinterpret_cast<unsigned int*>(u.Content);
			break;
		}
	}
}
void Mesh::Render(Shader* UsedShader, bool MainFrameBuffer)
{
	MeshVertexBuffer->Bind();
	MeshIndexBuffer->Bind();
	int TexIterator = 0;
	for (int i = 0; i < Uniforms.size(); ++i)
	{
		switch (Uniforms.at(i).Type)
		{
		case Type::E_INT:
			glUniform1iv(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), 1, static_cast<int*>(Uniforms.at(i).Content));
			break;
		case Type::E_FLOAT:
			glUniform1fv(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), 1, (float*)Uniforms[i].Content);
			break;
		case Type::E_VECTOR3:
			glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), static_cast<Vector3*>((Uniforms.at(i).Content))->X, static_cast<Vector3*>((Uniforms.at(i).Content))->Y, static_cast<Vector3*>((Uniforms.at(i).Content))->Z);
			break;
		case Type::E_GL_TEXTURE:
			glActiveTexture(GL_TEXTURE7 + TexIterator);
			glBindTexture(GL_TEXTURE_2D, *(unsigned int*)Uniforms.at(i).Content);
			glUniform1i(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), 7 + TexIterator);
			TexIterator++;
			break;
		default:
			break;
		}
	}
	if (MainFrameBuffer)
	{
		unsigned int attachements[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachements);
	}
	else
	{
		unsigned int attachements[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachements);
	}
	glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);
	MeshVertexBuffer->Unbind();
	MeshIndexBuffer->Unbind();
}
void Mesh::SimpleRender(Shader* UsedShader)
{
	if (MeshMaterial.UseShadowCutout)
	{
		glUniform1i(glGetUniformLocation(UsedShader->GetShaderID(), "u_usetexture"), 1);
		for (int i = 0; i < Uniforms.size(); ++i)
		{
			switch (Uniforms.at(i).Type)
			{
			case Type::E_INT:
				glUniform1iv(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), 1, static_cast<int*>(Uniforms.at(i).Content));
				break;
			case Type::E_FLOAT:
				glUniform1fv(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), 1, static_cast<float*>(Uniforms.at(i).Content));
				break;
			case Type::E_VECTOR3:
				glUniform3f(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), static_cast<Vector3*>((Uniforms.at(i).Content))->X, static_cast<Vector3*>((Uniforms.at(i).Content))->Y, static_cast<Vector3*>((Uniforms.at(i).Content))->Z);
				break;
			case Type::E_GL_TEXTURE:
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, *(unsigned int*)Uniforms.at(i).Content);
				glUniform1i(glGetUniformLocation(UsedShader->GetShaderID(), Uniforms.at(i).Name.c_str()), 7);
				break;
			default:
				break;
			}
		}
	}
	else glUniform1i(glGetUniformLocation(UsedShader->GetShaderID(), "u_usetexture"), 0);
	MeshVertexBuffer->Bind();
	MeshIndexBuffer->Bind();
	glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);
	MeshVertexBuffer->Unbind();
	MeshIndexBuffer->Unbind();
}

void Mesh::ApplyUniforms()
{
	for (size_t i = 0; i < Uniforms.size(); i++)
	{
		ApplyUniform(i);
	}
}

void Mesh::ApplyUniform(size_t Index)
{
	Uniform u = Uniforms.at(Index);
	char* PreviousContent = static_cast<char*>(u.Content);
	switch (u.Type)
	{
	case Type::E_INT:
		u.Content = new int(std::stoi(static_cast<char*>(u.Content)));
		break;
	case Type::E_FLOAT:
		u.Content = new float(std::stof(static_cast<char*>(u.Content)));
		break;
	case Type::E_VECTOR3:
		u.Content = new Vector3(Vector3::stov(static_cast<char*>(u.Content)));
		break;
	case Type::E_GL_TEXTURE:
		u.Content = (void*)new unsigned int(Texture::LoadTexture(std::string(static_cast<char*>(u.Content))));
		break;
	default:
		break;
	}
	Uniforms[Index] = u;
}
