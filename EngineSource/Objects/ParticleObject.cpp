#include "ParticleObject.h"
#include <Objects/Components/ParticleComponent.h>
#include <Engine/Stats.h>
#include <iostream>

void ParticleObject::Begin()
{
	AddEditorProperty(Property("Emitter", Type::String, &ParticleName));
	if (Particle) Detach(Particle);
	Particle = new ParticleComponent();
	Attach(Particle);
}

void ParticleObject::Destroy()
{
}

void ParticleObject::Tick()
{
	if (Particle->GetIsFinished() && !IsInEditor)
	{
		Objects::DestroyObject(this);
	}
}

void ParticleObject::OnPropertySet()
{
	Particle->LoadParticle(ParticleName);
}

void ParticleObject::LoadParticle(std::string ParticleName)
{
	this->ParticleName = ParticleName;
	Particle->LoadParticle(ParticleName);
}
