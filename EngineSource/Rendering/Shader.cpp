#define _CRT_SECURE_NO_WARNINGS
#include "Shader.h"
#include <fstream>
#include <iostream>
#include <Utility/stb_image.hpp>
#include <sstream>
#include <Engine/Log.h>
#include <Engine/Build/Pack.h>
#include <GL/glew.h>
#include <Math/Vector.h>
#include <Rendering/Utility/ShaderPreprocessor.h>
#include <glm/mat4x4.hpp>
#include <filesystem>


extern const bool IsInEditor;
extern const bool EngineDebug;

Shader::Shader(std::string VertexShaderFilename, std::string FragmentShaderFilename, std::string GeometryShader)
{
	ShaderID = CreateShader(VertexShaderFilename.c_str(), FragmentShaderFilename.c_str(), GeometryShader.empty() ? nullptr : GeometryShader.c_str());
	VertexFileName = VertexShaderFilename;
	FragmetFileName = FragmentShaderFilename;
}

Shader::~Shader()
{
	glDeleteProgram(ShaderID);
}

void Shader::Bind()
{
	glUseProgram(ShaderID);
}

void Shader::Unbind()
{
	glUseProgram(0);
}

void Shader::SetInt(std::string Field, int Value)
{
	glUniform1i(glGetUniformLocation(ShaderID, Field.c_str()), Value);
}

void Shader::SetFloat(std::string Field, float Value)
{
	glUniform1f(glGetUniformLocation(ShaderID, Field.c_str()), Value);
}

void Shader::SetVector4(std::string Field, Vector4 Value)
{
	glUniform4f(glGetUniformLocation(ShaderID, Field.c_str()), Value.X, Value.Y, Value.Z, Value.W);
}

void Shader::SetVector3(std::string Field, Vector3 Value)
{
	glUniform3f(glGetUniformLocation(ShaderID, Field.c_str()), Value.X, Value.Y, Value.Z);
}

void Shader::SetVector2(std::string Field, Vector2 Value)
{
	glUniform2f(glGetUniformLocation(ShaderID, Field.c_str()), Value.X, Value.Y);
}

void Shader::SetMat4(std::string Field, glm::mat4 Value)
{
	glUniformMatrix4fv(glGetUniformLocation(ShaderID, Field.c_str()), 1, GL_FALSE, &Value[0][0]);
}

GLuint Shader::Compile(std::string ShaderCode, unsigned int Type)
{
	GLuint id = glCreateShader(Type);
	const char* src = ShaderCode.c_str();
	glShaderSource(id, 1, &src, 0);

	glCompileShader(id);

	int Result;

	glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
	if (Result != GL_TRUE)
	{
		int length = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = new char[length];
		glGetShaderInfoLog(id, length, &length, message);
		Log::Print(std::string("Warning: OpenGL Shader Compile Error : ").append(message));
		delete[] message;
		return 0;
	}
	return id;
}

std::string Shader::parse(const char* Filename)
{
	FILE* File;
	File = fopen(Filename, "rb");
	if (File == nullptr)
	{
		Log::Print("File " + std::string(Filename) + " could not be found.");
		return std::string("");
	}
	std::string ShaderCode;
	fseek(File, 0, SEEK_END);
	size_t FileSize = ftell(File);
	rewind(File);
	ShaderCode.resize(FileSize);
	fread(&ShaderCode[0], 1, FileSize, File);
	fclose(File);

	return ShaderCode;
}

GLuint Shader::CreateShader(const char* VertexShader, const char* FragmentShader, const char* GeometryShader)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::string sharedCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream fSharedFile;
	std::ifstream gShaderFile;

	std::vector<std::string> Paths = { VertexShader, FragmentShader };
	if (GeometryShader != nullptr)
	{
		Paths.push_back(GeometryShader);
	}
	for (auto& i : Paths)
	{
		i = i.substr(0, i.find_last_of("/\\"));
	}

	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fSharedFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

#if !RELEASE
	if (EngineDebug || IsInEditor || !std::filesystem::exists("Assets"))
	{
		// open files
		vShaderFile.open(VertexShader);
		fShaderFile.open(FragmentShader);
		std::stringstream vShaderStream, fShaderStream, fSharedStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = Preprocessor::ParseGLSL(vShaderStream.str(), Paths[0]).Code;
		fragmentCode = Preprocessor::ParseGLSL(fShaderStream.str(), Paths[1]).Code;
		// if geometry shader path is present, also load a geometry shader
		if (GeometryShader != nullptr)
		{
			gShaderFile.open(GeometryShader);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = Preprocessor::ParseGLSL(gShaderStream.str(), Paths[2]).Code;
		}
	}
	else
#endif
	{
		vertexCode = Preprocessor::ParseGLSL(Pack::GetFile(VertexShader), Paths[0]).Code;
		fragmentCode = Preprocessor::ParseGLSL(Pack::GetFile(FragmentShader), Paths[1]).Code;
		// if geometry shader path is present, also load a geometry shader
		if (GeometryShader != nullptr)
		{
			geometryCode = Preprocessor::ParseGLSL(Pack::GetFile(GeometryShader), Paths[2]).Code;
		}
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX", VertexShader);
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT", FragmentShader);
	// fragment common functions Shader
	// if geometry shader is given, compile geometry shader
	unsigned int geometry;
	if (GeometryShader != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);

		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY", GeometryShader);
	}

	// shader Program
	ShaderID = glCreateProgram();
	glAttachShader(ShaderID, vertex);
	glAttachShader(ShaderID, fragment);
	if (GeometryShader != nullptr)
		glAttachShader(ShaderID, geometry);
	glLinkProgram(ShaderID);
	checkCompileErrors(ShaderID, "PROGRAM", (VertexShader + std::string("-") + FragmentShader));
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (GeometryShader != nullptr)
		glDeleteShader(geometry);
	return ShaderID;
}
void Shader::checkCompileErrors(unsigned int shader, std::string type, std::string ShaderName)
{
	GLint success;
	GLchar infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			Log::PrintMultiLine("ERROR::SHADER_COMPILATION_ERROR: " + ShaderName + " :TYPE " + type + "\n INFO LOG: \n" + std::string(infoLog)
				+ "\n -- --------------------------------------------------- -- ", Log::LogColor::Red, "[Error]: ");
			throw "Shader compile error";
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			Log::PrintMultiLine("ERROR::PROGRAM_LINKING_ERROR of type: " + ShaderName + " :TYPE " + type + "\n INFO LOG: \n" + std::string(infoLog)
				+ "\n -- --------------------------------------------------- -- ", Log::LogColor::Red, "[Error]: ");
			throw "Shader compile error";
		}
	}
}