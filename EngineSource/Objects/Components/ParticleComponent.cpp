#include "ParticleComponent.h"
#include <filesystem>
#include <Rendering/Particle.h>
#include <World/Assets.h>
#include <Rendering/Mesh/Model.h>
#include <World/Graphics.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Log.h>
#include <Rendering/Mesh/Mesh.h>

void ParticleComponent::Begin()
{
	Emitter = new Particles::ParticleEmitter();
	Graphics::MainFramebuffer->ParticleEmitters.push_back(Emitter);
}

void ParticleComponent::Tick()
{
	Emitter->Position = Vector3::TranslateVector(Position, GetParent()->GetTransform());
	Emitter->Rotation = Rotation;
}
void ParticleComponent::Destroy()
{
	for (auto f : Graphics::AllFramebuffers)
	{
		for (int i = 0; i < f->ParticleEmitters.size(); i++)
		{
			if (f->ParticleEmitters[i] == Emitter)
			{
				f->ParticleEmitters.erase(f->ParticleEmitters.begin() + i);
			}
		}
	}
	delete Emitter;
}
void ParticleComponent::LoadParticle(std::string Name)
{
	std::vector<std::string> ElementMaterials;
	std::string File = Assets::GetAsset(Name + ".jspart");
	for (unsigned int i = 0; i < Emitter->ParticleVertexBuffers.size(); i++)
	{
		delete Emitter->ParticleVertexBuffers[i];
	}
	Emitter->ParticleVertexBuffers.clear();
	Emitter->SpawnDelays.clear();
	Emitter->Contexts.clear();
	Emitter->ParticleInstances.clear();
	Emitter->ParticleMatrices.clear();
	Emitter->ParticleElements.clear();

	if (std::filesystem::exists(File) && !std::filesystem::is_empty(File))
	{
		auto ParticleData = Particles::ParticleEmitter::LoadParticleFile(File, ElementMaterials);
		for (unsigned int i = 0; i < ParticleData.size(); i++)
		{
			Emitter->AddElement(ParticleData[i], Material::LoadMaterialFile(ElementMaterials[i], false));
		}
	}
	else if (!std::filesystem::exists(File))
	{
		Log::Print("Particle emitter " + Name + " does not exist");
	}
}
void ParticleComponent::SetActive(bool Active)
{
	Emitter->Active = Active;
}
bool ParticleComponent::GetActive()
{
	return Emitter->Active;
}
bool ParticleComponent::GetIsFinished()
{
	return !Emitter->IsActive;
}
void ParticleComponent::Reset()
{
	Emitter->Reset();
}
void ParticleComponent::SetRelativeRotation(Vector3 NewRotation)
{
	this->Rotation = NewRotation;
}
Vector3 ParticleComponent::GetRelativeRotation()
{
	return Rotation;
}
void ParticleComponent::SetRelativePosition(Vector3 NewPos)
{
	Position = NewPos;
}
Vector3 ParticleComponent::GetRelativePosition()
{
	return Position;
}