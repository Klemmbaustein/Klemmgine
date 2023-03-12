#pragma once
#include <string>
#include <iostream>
struct Shader
{
	Shader(const char* VertexShaderFilename, const char* FragmentShaderFilename, const char* GeometryShader = nullptr);
	virtual ~Shader();

	void Bind();
	void Unbind();

	int GetShaderID()
	{
		return ShaderID;
	}


	void Recompile();

private:
	unsigned int Compile(std::string ShaderCode, unsigned int Type);
	std::string parse(const char* Filename);
	unsigned int CreateShader(const char* VertexShader, const char* FragmentShader, const char* GeometryShader);
	void checkCompileErrors(unsigned int shader, std::string type, std::string ShaderName);

	std::string VertexFileName, FragmetFileName;

	unsigned int ShaderID;
};