#include "Particle.h"
#include <Rendering/VertexBuffer.h>
#include <GL/glew.h>
#include <Rendering/Utility/IndexBuffer.h>
#include <glm/ext/matrix_transform.hpp>
#include <Rendering/Shader.h>
#include <World/Stats.h>
#include <Math/Math.h>
#include <Engine/EngineRandom.h>
#include <Rendering/Utility/ShaderManager.h>
#include <fstream>
#include <filesystem>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Texture/Material.h>
#include <Rendering/Texture/Texture.h>
#include <Rendering/Camera/Camera.h>
#include <World/Assets.h>
#include <World/Graphics.h>
#include <Engine/Log.h>

struct Uniform
{
	std::string Name;
	int Type;
	void* Content;
	Uniform(std::string Name, int Type, void* Content)
	{
		this->Content = Content;
		this->Name = Name;
		this->Type = Type;
	}
};

void Particles::ParticleEmitter::AddElement(ParticleElement NewElement)
{
	ParticleElements.push_back(NewElement);
	SpawnDelays.push_back(NewElement.SpawnDelay);
	ParticleInstances.push_back(std::vector<ParticleInstance>());
	std::vector<Vertex> ParticleVertices;
	Vertex Vert = Vertex();
	Vert.Position = glm::vec3(0.5, 0, -0.5);
	Vert.U = 0; Vert.V = 0;
	ParticleVertices.push_back(Vert);

	Vert.Position = glm::vec3(0.5, 0, 0.5);
	Vert.U = 1; Vert.V = 0;
	ParticleVertices.push_back(Vert);

	Vert.Position = glm::vec3(-0.5, 0, -0.5);
	Vert.U = 0; Vert.V = 1;
	ParticleVertices.push_back(Vert);

	Vert.Position = glm::vec3(-0.5, 0, 0.5);
	Vert.U = 1; Vert.V = 1;
	ParticleVertices.push_back(Vert);
	ParticleVertexBuffers.push_back(new VertexBuffer(ParticleVertices.data(), ParticleVertices.size()));
	std::vector<int> ParticleIndices =
	{
		0, 2, 1,
		1, 2, 3
	};
	ParticleIndexBuffers.push_back(new IndexBuffer(ParticleIndices.data(), ParticleIndices.size(), sizeof(int)));
	ParticleShaders.push_back(nullptr);
	Uniforms.push_back(std::vector<Uniform>());
	if (IsInEditor || EngineDebug)
	{
		SetMaterial(Uniforms.size() - 1, "NONE", true);
	}
}

std::vector<Particles::ParticleElement> Particles::ParticleEmitter::LoadParticleFile(std::string File, std::vector<std::string>& Materials)
{
	std::ifstream InF = std::ifstream(File, std::ios::in);
	std::vector<Particles::ParticleElement> Data;
	uint8_t NumElements;
	InF.read((char*)&NumElements, sizeof(uint8_t));
	for (uint8_t i = 0; i < NumElements; i++)
	{
		ParticleElement elem;
		InF.read((char*)&elem, sizeof(ParticleElement));
		elem.RunLoops = elem.NumLoops;
		Data.push_back(elem);
	}
	Materials.clear();
	InF.read((char*)&NumElements, sizeof(uint8_t));
	for (uint8_t i = 0; i < NumElements; i++)
	{
		size_t len;
		InF.read((char*)&len, sizeof(size_t));
		char* temp = new char[len + 1];
		InF.read(temp, len);
		temp[len] = '\0';
		Materials.push_back(temp);
		delete[] temp;
	}
	InF.close();
	return Data;
}

void Particles::ParticleEmitter::SaveToFile(std::vector<ParticleElement> Data, std::vector<std::string> Materials, std::string File)
{
	std::ofstream OutF = std::ofstream(File, std::ios::out);

	uint8_t NumElements = Data.size();
	OutF.write((char*)&NumElements, sizeof(uint8_t));
	for (uint8_t i = 0; i < NumElements; i++)
	{
		OutF.write((char*)&Data[i], sizeof(ParticleElement));
	}
	NumElements = Materials.size();
	OutF.write((char*)&NumElements, sizeof(uint8_t));
	for (uint8_t i = 0; i < NumElements; i++)
	{
		size_t size = Materials[i].size();

		OutF.write((char*)&size, sizeof(size_t));
		OutF.write(Materials[i].c_str(), sizeof(char) * size);
	}
	OutF.close();
}

void Particles::ParticleEmitter::UpdateParticlePositions(Camera* MainCamera)
{
	for (unsigned int i = 0; i < ParticleVertexBuffers.size(); i++)
	{
		ParticleMatrices.clear();
		for (const auto& T : ParticleInstances[i])
		{
			glm::mat4 Inst = glm::mat4(1.f);
			Inst = glm::translate(Inst, (glm::vec3)(Vector3::TranslateVector(T.Position, Transform(0, Rotation.DegreesToRadiants(), 1)) + Position));
			Vector3 Rotation = Vector3() - MainCamera->Rotation.DegreesToRadiants();
			Inst = glm::rotate(Inst, Rotation.Y, glm::vec3(0, 1, 0));
			Inst = glm::rotate(Inst, (float)Maths::PI, glm::vec3(0, 1, 0));
			Inst = glm::rotate(Inst, Rotation.X + -(float)Maths::PI / 2.f, glm::vec3(0, 0, 1));

			float TimePercentage = T.LifeTime / T.InitialLifeTime;
			float ScaleMultiplier = std::lerp(T.EndScale, T.StartScale, TimePercentage);
			
			ParticleMatrices.push_back(glm::scale(Inst, (glm::vec3)(T.Scale * ScaleMultiplier)));
		}
		if (MatBuffer != -1)
			glDeleteBuffers(1, &MatBuffer);
		glGenBuffers(1, &MatBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
		glBufferData(GL_ARRAY_BUFFER, ParticleMatrices.size() * sizeof(glm::mat4), &ParticleMatrices[i], GL_DYNAMIC_DRAW);
		unsigned int VAO = ParticleVertexBuffers[i]->GetVAO();
		glBindVertexArray(VAO);
		// vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);

		glBindVertexArray(0);
	}
}

void Particles::ParticleEmitter::AddParticleInstance(unsigned int Element)
{
	if (ParticleElements[Element].RunLoops != 0 && Active)
	{
		auto NewP = ParticleInstance();
		NewP.LifeTime = ParticleElements[Element].LifeTime;
		NewP.InitialLifeTime = ParticleElements[Element].LifeTime;
		NewP.Position = Vector3(0);
		NewP.Scale = ParticleElements[Element].Size;
		Vector3 Rand = ParticleElements[Element].DirectionRandom;
		Rand.X *= Random::GetRandomNumber(-1.f, 1.f);
		Rand.Y *= Random::GetRandomNumber(-1.f, 1.f);
		Rand.Z *= Random::GetRandomNumber(-1.f, 1.f);
		NewP.Velocity = ParticleElements[Element].Direction + Rand;
		Rand = ParticleElements[Element].PositionRandom;
		Rand.X *= Random::GetRandomNumber(-1.f, 1.f);
		Rand.Y *= Random::GetRandomNumber(-1.f, 1.f);
		Rand.Z *= Random::GetRandomNumber(-1.f, 1.f);
		NewP.Position = Rand;
		NewP.Force = ParticleElements[Element].Force;
		NewP.StartScale = ParticleElements[Element].StartScale;
		NewP.EndScale = ParticleElements[Element].EndScale;
		ParticleInstances[Element].push_back(NewP);
		if(ParticleElements[Element].RunLoops > 0)
		ParticleElements[Element].RunLoops--;
	}
}

void Particles::ParticleEmitter::ApplyUniforms(unsigned int Element)
{
	for (size_t i = 0; i < Uniforms[Element].size(); i++)
	{
		ApplyUniform(i, Element);
	}
}

void Particles::ParticleEmitter::ApplyUniform(int Index, unsigned int Element)
{
	{
		Uniform u = Uniforms[Element][Index];
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
		Uniforms[Element][Index] = u;
	}
}

Particles::ParticleEmitter::ParticleEmitter()
{
}

#define REMOVE_ARRAY_IND(Arr, Ind) Arr.erase(Arr.begin() + Ind)
void Particles::ParticleEmitter::RemoveElement(unsigned int Index)
{
	REMOVE_ARRAY_IND(ParticleElements, Index);
	REMOVE_ARRAY_IND(SpawnDelays, Index);
	ParticleInstances.push_back(std::vector<ParticleInstance>());

	delete ParticleVertexBuffers[Index];
	delete ParticleIndexBuffers[Index];
	
	REMOVE_ARRAY_IND(ParticleVertexBuffers, Index);
	REMOVE_ARRAY_IND(ParticleIndexBuffers, Index);

	REMOVE_ARRAY_IND(ParticleShaders, Index);
	REMOVE_ARRAY_IND(Uniforms, Index);
}

Particles::ParticleEmitter::~ParticleEmitter()
{
	for (auto* buf : ParticleVertexBuffers)
	{
		delete buf;
	}
	for (auto* buf : ParticleIndexBuffers)
	{
		delete buf;
	}
}

void Particles::ParticleEmitter::SetMaterial(unsigned int Index, std::string NewMaterial, bool SupressMaterialError)
{
	if (!(Index < Uniforms.size()))
	{
		return;
	}
	if (!std::filesystem::exists(NewMaterial))
	{
		std::string MaterialAsset = Assets::GetAsset(NewMaterial + ".jsmat");

		if (std::filesystem::exists(MaterialAsset))
		{
			NewMaterial = MaterialAsset;
		}
		if (!std::filesystem::exists(NewMaterial))
		{
			if (IsInEditor || EngineDebug)
			{
				if(!SupressMaterialError)
					Log::Print("Material given by the mesh does not exist. Falling back to default phong material - " + NewMaterial, Vector3(1, 1, 0));
			}
			else
			{
				Log::Print("Material " + NewMaterial + " does not exist. Fallback material only avaliable with editor.");
				throw "Material does not exist!";
			}
			NewMaterial = "EditorContent/Materials/EngineDefaultPhong.jsmat";
		}
	}
	Material mat;
	try
	{
		if (!std::filesystem::exists(NewMaterial))
		{
			return;
		}

		mat = Material::LoadMaterialFile(NewMaterial, false);
		ParticleShaders[Index] = ReferenceShader("Shaders/" + mat.VertexShader, "Shaders/" + mat.FragmentShader);
		if (ParticleShaders[Index] == nullptr)
		{
			ParticleShaders[Index] = ReferenceShader("Shaders/basic.vert", "Shaders/basic.frag");
			Log::Print("Invalid Shader");
		}

	}
	catch (std::exception& e)
	{
		Log::Print(e.what(), Vector3(0.8f, 0.f, 0.f));
	}
	try
	{
		Uniforms[Index].clear();
		for (int i = 0; i < mat.Uniforms.size(); i++)
		{
			Uniforms[Index].push_back(Uniform("", 0, nullptr));
			Uniforms[Index].at(i).Name = mat.Uniforms.at(i).UniformName;
			Uniforms[Index].at(i).Type = mat.Uniforms.at(i).Type;
			Uniforms[Index].at(i).Content = (void*)mat.Uniforms.at(i).Value.c_str();
		}
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}
	ApplyUniforms(Index);
}

void Particles::ParticleEmitter::Reset()
{
	for (auto& Elem : ParticleElements)
	{
		Elem.RunLoops = Elem.NumLoops;
	}
	//for (auto& inst : ParticleInstances)
	//{
	//	inst.clear();
	//}
	for (unsigned int i = 0; i < ParticleElements.size(); i++)
	{
		SpawnDelays[i] = ParticleElements[i].SpawnDelay;
	}
}

void Particles::ParticleEmitter::Update(Camera* MainCamera)
{
	IsActive = false;
	for (unsigned int i = 0; i < ParticleElements.size(); i++)
	{
		SpawnDelays[i] -= Performance::DeltaTime;
		if (SpawnDelays[i] < 0.0f)
		{
			AddParticleInstance(i);
			SpawnDelays[i] = ParticleElements[i].SpawnDelay;
		}
		if (ParticleElements[i].RunLoops != 0 || ParticleInstances[i].size() > 0)
		{
			IsActive = true;
		}
	}
	for (unsigned int elem = 0; elem < ParticleInstances.size(); elem++)
	{
		std::vector<ParticleInstance> ParticlesToDelete;
		for (auto& p : ParticleInstances[elem])
		{
			p.Position += p.Velocity * Performance::DeltaTime;
			p.Velocity += p.Force * Performance::DeltaTime;
			p.LifeTime -= Performance::DeltaTime;
			if (p.LifeTime < 0.f)
			{
				ParticlesToDelete.push_back(p);
			}
		}

		for (auto& i : ParticlesToDelete)
		{
			unsigned int it = 0;
			for (auto& j : ParticleInstances[elem])
			{
				if (i == j)
				{
					ParticleInstances[elem].erase(ParticleInstances[elem].begin() + it);
				}
				it++;
			}
		}
	}
	UpdateParticlePositions(MainCamera);
}

void Particles::ParticleEmitter::Draw(Camera* MainCamera , bool MainFrameBuffer, bool TransparencyPass)
{
	if (!TransparencyPass) return;
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
	for (unsigned int Elem = 0; Elem < ParticleElements.size(); Elem++)
	{
		ParticleShaders[Elem]->Bind();
		glUniformMatrix4fv(glGetUniformLocation(ParticleShaders[Elem]->GetShaderID(), "u_projection"), 1, GL_FALSE, &MainCamera->GetProjection()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(ParticleShaders[Elem]->GetShaderID(), "u_viewpro"), 1, GL_FALSE, &MainCamera->getViewProj()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(ParticleShaders[Elem]->GetShaderID(), "u_view"), 1, GL_FALSE, &MainCamera->getView()[0][0]); uint8_t TexIterator = 0;
		for (int i = 0; i < Uniforms[Elem].size(); ++i)
		{
			switch (Uniforms[Elem][i].Type)
			{
			case Type::E_INT:
				glUniform1iv(glGetUniformLocation(ParticleShaders[Elem]->GetShaderID(), Uniforms[Elem][i].Name.c_str()), 1, static_cast<int*>(Uniforms[Elem][i].Content));
				break;
			case Type::E_FLOAT:
				glUniform1fv(glGetUniformLocation(ParticleShaders[Elem]->GetShaderID(), Uniforms[Elem][i].Name.c_str()), 1, (float*)Uniforms[Elem][i].Content);
				break;
			case Type::E_VECTOR3:
				glUniform3f(glGetUniformLocation(ParticleShaders[Elem]->GetShaderID(), Uniforms[Elem][i].Name.c_str()),
					static_cast<Vector3*>((Uniforms[Elem][i].Content))->X,
					static_cast<Vector3*>((Uniforms[Elem][i].Content))->Y,
					static_cast<Vector3*>((Uniforms[Elem][i].Content))->Z);
				break;
			case Type::E_GL_TEXTURE:
				glActiveTexture(GL_TEXTURE7 + TexIterator);
				glBindTexture(GL_TEXTURE_2D, *(unsigned int*)Uniforms[Elem][i].Content);
				glUniform1i(glGetUniformLocation(ParticleShaders[Elem]->GetShaderID(), Uniforms[Elem][i].Name.c_str()), 7 + TexIterator);
				TexIterator++;
				break;
			default:
				break;
			}
		}
		ParticleVertexBuffers[Elem]->Bind();
		ParticleIndexBuffers[Elem]->Bind();
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
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, ParticleInstances[Elem].size());
		ParticleVertexBuffers[Elem]->Unbind();
		ParticleIndexBuffers[Elem]->Unbind();
	}
}
