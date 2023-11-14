#include "ParticleComponent.h"
#include <filesystem>
#include <Rendering/Particle.h>
#include <Engine/File/Assets.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Graphics.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Log.h>
#include <Rendering/Mesh/Mesh.h>

void ParticleComponent::Begin()
{
#if !SERVER
	Emitter = new Particles::ParticleEmitter();
	Graphics::MainFramebuffer->ParticleEmitters.push_back(Emitter);
#endif
}

void ParticleComponent::Tick()
{
#if !SERVER
	Emitter->Position = Vector3::TranslateVector(Position, GetParent()->GetTransform());
	Emitter->Rotation = Rotation;
#endif
}
void ParticleComponent::Destroy()
{
#if !SERVER
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
#endif
}
void ParticleComponent::LoadParticle(std::string Name)
{
#if !SERVER
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
			Emitter->AddElement(ParticleData[i], Material::LoadMaterialFile(ElementMaterials[i]));
		}
	}
	else if (!std::filesystem::exists(File))
	{
		Log::Print("Particle emitter " + Name + " does not exist");
	}
#endif
}
void ParticleComponent::SetActive(bool Active)
{
#if !SERVER
	Emitter->Active = Active;
#endif
}
bool ParticleComponent::GetActive()
{
#if !SERVER
	return Emitter->Active;
#endif
	return false;
}
bool ParticleComponent::GetIsFinished()
{
#if !SERVER
	return !Emitter->IsActive;
#endif
	return false;
}
void ParticleComponent::Reset()
{
#if !SERVER
	Emitter->Reset();
#endif
}
void ParticleComponent::SetRelativeRotation(Vector3 NewRotation)
{
#if !SERVER
	this->Rotation = NewRotation;
#endif
}
Vector3 ParticleComponent::GetRelativeRotation()
{
#if !SERVER
	return Rotation;
#endif
	return 0;
}
void ParticleComponent::SetRelativePosition(Vector3 NewPos)
{
#if !SERVER
	Position = NewPos;
#endif
}
Vector3 ParticleComponent::GetRelativePosition()
{
#if !SERVER
	return Position;
#endif
	return 0;
}