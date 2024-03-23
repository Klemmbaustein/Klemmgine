#pragma once
#include <vector>
#include <map>
#include <Rendering/Shader.h>

namespace ShaderManager
{
	struct ShaderDescription
	{
		std::string VertexShader;
		std::string FragmentShader;
	};


	struct ShaderElement
	{
		ShaderElement()
		{

		}
		ShaderElement(Shader* UsedShader, int References = 1)
		{
			this->UsedShader = UsedShader;
			this->References = References;
		}
		Shader* UsedShader = nullptr;
		int References = 0;
	};

	bool operator<(ShaderDescription a, ShaderDescription b);

	extern std::map<ShaderDescription, ShaderElement> Shaders;

	Shader* ReferenceShader(std::string VertexShader, std::string FragmentShader); /*Creates shader from the given files. if a shader with the same source files already exists,

																				   it returns that one, since there is no need for duplicate shaders*/

	size_t GetNumShaders();

	Shader* GetShader(std::string VertexShader, std::string FragmentShader); //Gets a pointer to the shader class from the given files. if the shader doesnt exists this returns nullptr;

	void DereferenceShader(std::string VertexShader, std::string FragmentShader); //Derefences Shader, if Count of References for that shader <= 0, the shader will be deleted.

	/// Derefences Shader. If the count of References for that shader <= 0, the shader will be deleted.
	void DereferenceShader(Shader* UsedShader);
}