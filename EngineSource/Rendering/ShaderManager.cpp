#include "ShaderManager.h"
#include <Engine/Log.h>
#include <filesystem>
#include <Engine/Stats.h>
#include <Engine/EngineError.h>

std::map<ShaderManager::ShaderDescription, ShaderManager::ShaderElement> ShaderManager::Shaders;


bool ShaderManager::operator<(ShaderDescription a, ShaderDescription b)
{
	return (a.VertexShader + a.FragmentShader) < (b.VertexShader + b.FragmentShader);
}

Shader* ShaderManager::ReferenceShader(std::string VertexShader, std::string FragmentShader)
{
	if ((!std::filesystem::exists(VertexShader) || !std::filesystem::exists(FragmentShader)) && (EngineDebug || IsInEditor))
	{
		Log::Print("Could not find shader: " + VertexShader + " - " + FragmentShader);
		return nullptr;
	}
	ShaderDescription ShaderToFind;
	ShaderToFind.FragmentShader = FragmentShader;
	ShaderToFind.VertexShader = VertexShader;
	auto FoundShader = Shaders.find(ShaderToFind);
	if (!Shaders.contains(ShaderToFind))
	{
		try
		{
			Shader* NewShader = new Shader(VertexShader.c_str(), FragmentShader.c_str());
			Shaders.insert(std::make_pair(ShaderToFind, ShaderElement(NewShader, 1)));
			return NewShader;
		}
		catch (const char* err)
		{
			Log::Print("-- " + std::string(err) + " --");

			Shader* FallbackShader = new Shader("Shaders/basic.vert", "Shaders/basic.frag");

			Shaders.insert(std::make_pair(ShaderToFind, ShaderElement(FallbackShader, 1)));

			return FallbackShader;
		}
	}
	else
	{
		FoundShader->second.References++;
		return FoundShader->second.UsedShader;
	}
}

size_t ShaderManager::GetNumShaders()
{
	return ShaderManager::Shaders.size();
}

Shader* ShaderManager::GetShader(std::string VertexShader, std::string FragmentShader)
{
	ShaderDescription ShaderToFind;
	ShaderToFind.FragmentShader = FragmentShader;
	ShaderToFind.VertexShader = VertexShader;
	auto FoundShader = Shaders.find(ShaderToFind);
	if (FoundShader == Shaders.end())
	{
		return nullptr;
	}
	return FoundShader->second.UsedShader;
}

void ShaderManager::DereferenceShader(std::string VertexShader, std::string FragmentShader)
{
	ShaderDescription ShaderToFind;
	ShaderToFind.FragmentShader = FragmentShader;
	ShaderToFind.VertexShader = VertexShader;
	auto FoundShader = Shaders.find(ShaderToFind);
	if (!Shaders.count(ShaderToFind))
	{
		//Log::Print("Error: Could not dereference Shader: " + VertexShader + " - " + FragmentShader, Vector3(1, 0, 0));
	}
	else
	{
		FoundShader->second.References--;
		if (FoundShader->second.References <= 0)
		{
			delete FoundShader->second.UsedShader;
			Shaders.erase(FoundShader);
		}
	}
}

void ShaderManager::DereferenceShader(Shader* UsedShader)
{
	for (auto it = Shaders.begin(); it->first < Shaders.end()->first; it++)
	{
		if (it->second.UsedShader == UsedShader)
		{
			it->second.References--;
			if (it->second.References <= 0)
			{
				delete it->second.UsedShader;
				Shaders.erase(it);
			}
		}
	}
	ENGINE_ASSERT(true, "Could not dereference a shader - It was never referenced!");
}
