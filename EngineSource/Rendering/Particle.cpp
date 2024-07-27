#if !SERVER
#include "Particle.h"
#include <Rendering/VertexBuffer.h>
#include <GL/glew.h>
#include <glm/ext/matrix_transform.hpp>
#include <Rendering/Shader.h>
#include <Engine/Stats.h>
#include <Math/Math.h>
#include <Engine/EngineRandom.h>
#include <Rendering/ShaderManager.h>
#include <fstream>
#include <filesystem>
#include <Rendering/Texture/Material.h>
#include <Rendering/Texture/Texture.h>
#include <Rendering/Camera/Camera.h>
#include <Engine/File/Assets.h>
#include <Rendering/Graphics.h>
#include <Engine/Log.h>
#include <Rendering/Drawable.h>

struct Uniform
{
	std::string Name;
	int NativeType;
	void* Content;
	Uniform(std::string Name, int NativeType, void* Content)
	{
		this->Content = Content;
		this->Name = Name;
		this->NativeType = NativeType;
	}
};

void Particles::ParticleEmitter::SetMaterial(unsigned int Index, Material Mat)
{
	Contexts[Index] = ObjectRenderContext(Mat);
}

void Particles::ParticleEmitter::AddElement(ParticleElement NewElement, Material Mat)
{
	ParticleElements.push_back(NewElement);
	SpawnDelays.push_back(NewElement.SpawnDelay);
	ParticleInstances.push_back(std::vector<ParticleInstance>());
	std::vector<Vertex> ParticleVertices;
	Vertex Vert = Vertex();
	Vert.Position = glm::vec3(0.5, 0, -0.5);
	Vert.TexCoord = glm::vec2(0, 0);
	ParticleVertices.push_back(Vert);

	Vert.Position = glm::vec3(0.5, 0, 0.5);
	Vert.TexCoord = glm::vec2(1, 0);
	ParticleVertices.push_back(Vert);

	Vert.Position = glm::vec3(-0.5, 0, -0.5);
	Vert.TexCoord = glm::vec2(0, 1);
	ParticleVertices.push_back(Vert);

	Vert.Position = glm::vec3(-0.5, 0, 0.5);
	Vert.TexCoord = glm::vec2(1, 1);
	ParticleVertices.push_back(Vert);
	ParticleVertexBuffers.push_back(new VertexBuffer(ParticleVertices, {0, 2, 1, 1, 2, 3}));
	Contexts.push_back(ObjectRenderContext(Mat));
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

	uint8_t NumElements = (uint8_t)Data.size();
	OutF.write((char*)&NumElements, sizeof(uint8_t));
	for (uint8_t i = 0; i < NumElements; i++)
	{
		OutF.write((char*)&Data[i], sizeof(ParticleElement));
	}
	NumElements = (uint8_t)Materials.size();
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
	if (ParticleInstances.empty())
	{
		return;
	}
	for (size_t i = 0; i < ParticleVertexBuffers.size(); i++)
	{
		ParticleMatrices.clear();
		for (const auto& T : ParticleInstances[i])
		{
			glm::mat4 Inst = glm::mat4(1.f);
			Inst = glm::translate(Inst, (glm::vec3)(Vector3::TranslateVector(T.Position, Transform(0, Rotation.DegreesToRadians(), 1)) + Position));
			Vector3 Rotation = Vector3() - MainCamera->Rotation.DegreesToRadians();
			Inst = glm::rotate(Inst, Rotation.Y, glm::vec3(0, 1, 0));
			Inst = glm::rotate(Inst, (float)Math::PI, glm::vec3(0, 1, 0));
			Inst = glm::rotate(Inst, Rotation.X + -(float)Math::PI / 2.f, glm::vec3(0, 0, 1));

			float TimePercentage = T.LifeTime / T.InitialLifeTime;
			float ScaleMultiplier = std::lerp(T.EndScale, T.StartScale, TimePercentage);
			
			ParticleMatrices.push_back(glm::scale(Inst, (glm::vec3)(T.Scale * ScaleMultiplier)));
		}
		if (MatBuffer != -1)
			glDeleteBuffers(1, &MatBuffer);
		glGenBuffers(1, &MatBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, MatBuffer);
		glBufferData(GL_ARRAY_BUFFER, ParticleMatrices.size() * sizeof(glm::mat4), &ParticleMatrices[i], GL_STATIC_DRAW);
		unsigned int VAO = ParticleVertexBuffers[i]->VAO;
		glBindVertexArray(VAO);
		// vertex attributes
		size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)0);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * (GLsizei)vec4Size, (void*)(3 * vec4Size));

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
		Rand.X *= Random::GetRandomFloat(-1.f, 1.f);
		Rand.Y *= Random::GetRandomFloat(-1.f, 1.f);
		Rand.Z *= Random::GetRandomFloat(-1.f, 1.f);
		NewP.Velocity = ParticleElements[Element].Direction + Rand;
		Rand = ParticleElements[Element].PositionRandom;
		Rand.X *= Random::GetRandomFloat(-1.f, 1.f);
		Rand.Y *= Random::GetRandomFloat(-1.f, 1.f);
		Rand.Z *= Random::GetRandomFloat(-1.f, 1.f);
		NewP.Position = Rand;
		NewP.Force = ParticleElements[Element].Force;
		NewP.StartScale = ParticleElements[Element].StartScale;
		NewP.EndScale = ParticleElements[Element].EndScale;
		ParticleInstances[Element].push_back(NewP);
		if (ParticleElements[Element].RunLoops > 0) ParticleElements[Element].RunLoops--;
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
	
	REMOVE_ARRAY_IND(ParticleVertexBuffers, Index);

	Contexts[Index].Unload();
	REMOVE_ARRAY_IND(Contexts, Index);
}

Particles::ParticleEmitter::~ParticleEmitter()
{
	for (auto* buf : ParticleVertexBuffers)
	{
		delete buf;
	}
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
	for (size_t i = 0; i < ParticleElements.size(); i++)
	{
		SpawnDelays[i] = ParticleElements[i].SpawnDelay;
	}
}

constexpr size_t MAX_PARTICLES_PER_FRAME = 15;

void Particles::ParticleEmitter::Update(Camera* MainCamera)
{
	IsActive = false;
	for (size_t i = 0; i < ParticleElements.size(); i++)
	{
		SpawnDelays[i] -= Stats::DeltaTime;
		if (SpawnDelays[i] < 0.0f)
		{
			if (ParticleElements[i].SpawnDelay == 0)
			{
				for (size_t j = 0; j < MAX_PARTICLES_PER_FRAME; j++)
				{
					AddParticleInstance(i);
					if (ParticleElements[i].RunLoops == 0)
					{
						break;
					}
				}
			}
			else
			{
				AddParticleInstance(i);
			}
			SpawnDelays[i] = ParticleElements[i].SpawnDelay;
		}
		if (ParticleElements[i].RunLoops != 0 || ParticleInstances[i].size() > 0)
		{
			IsActive = true;
		}
	}
	for (size_t elem = 0; elem < ParticleInstances.size(); elem++)
	{
		std::vector<ParticleInstance> ParticlesToDelete;
		for (auto& p : ParticleInstances[elem])
		{
			p.Position += p.Velocity * Stats::DeltaTime;
			p.Velocity += p.Force * Stats::DeltaTime;
			p.LifeTime -= Stats::DeltaTime;
			if (p.LifeTime < 0.f)
			{
				ParticlesToDelete.push_back(p);
			}
		}

		for (auto& i : ParticlesToDelete)
		{
			size_t it = 0;
			for (auto& j : ParticleInstances[elem])
			{
				if (i == j)
				{
					ParticleInstances[elem].erase(ParticleInstances[elem].begin() + it);
					break;
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
	for (size_t Elem = 0; Elem < ParticleElements.size(); Elem++)
	{
		unsigned int ElemShader = Contexts[Elem].GetShader()->GetShaderID();
		Contexts[Elem].Bind();
		glUniformMatrix4fv(glGetUniformLocation(ElemShader, "u_projection"), 1, GL_FALSE, &MainCamera->GetProjection()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(ElemShader, "u_viewpro"), 1, GL_FALSE, &MainCamera->getViewProj()[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(ElemShader, "u_view"), 1, GL_FALSE, &MainCamera->getView()[0][0]);
		ParticleVertexBuffers[Elem]->Bind();
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
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (GLsizei)ParticleInstances[Elem].size());
		ParticleVertexBuffers[Elem]->Unbind();
	}
}
#endif