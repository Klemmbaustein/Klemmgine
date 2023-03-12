#include "ParticleObject.h"
#include <Objects/Components/ParticleComponent.h>
#include <World/Stats.h>
#include <iostream>

void ParticleObject::Begin()
{
	Properties.push_back(Objects::Property("Emitter", Type::E_STRING, &ParticleName));
	if (Particle) Detach(Particle);
	Particle = new ParticleComponent();
	Attach(Particle);
}

void ParticleObject::Destroy()
{
}

void ParticleObject::Tick()
{
	Particle->SetRelativeRotation(GetTransform().Rotation);
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
