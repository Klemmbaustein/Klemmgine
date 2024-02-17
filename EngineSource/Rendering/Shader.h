#pragma once
#include <string>
#include <iostream>
#include <glm/fwd.hpp>

struct Vector4;
struct Vector3;
struct Vector2;

struct Shader
{
	Shader(std::string VertexShaderFilename, std::string FragmentShaderFilename, std::string GeometryShader = "");
	virtual ~Shader();

	void Bind();
	void Unbind();

	int GetShaderID()
	{
		return ShaderID;
	}

	void SetInt(std::string Field, int Value);
	void SetFloat(std::string Field, float Value);
	void SetVector4(std::string Field, Vector4 Value);
	void SetVector3(std::string Field, Vector3 Value);
	void SetVector2(std::string Field, Vector2 Value);
	void SetMat4(std::string Field, glm::mat4 Value);

private:
	unsigned int Compile(std::string ShaderCode, unsigned int NativeType);
	std::string parse(const char* Filename);
	unsigned int CreateShader(const char* VertexShader, const char* FragmentShader, const char* GeometryShader);
	void checkCompileErrors(unsigned int shader, std::string type, std::string ShaderName);

	std::string VertexFileName, FragmetFileName;

	unsigned int ShaderID;
};