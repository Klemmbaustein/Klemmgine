#include "PointLightComponent.h"
#include <Engine/Log.h>

void PointLightComponent::Begin()
{
	Graphics::Lights.push_back(CurrentLight);
}

void PointLightComponent::Tick()
{
	if (CurrentLight != PreviousLight || PreviousTransform != GetParent()->GetTransform())
	{
		CurrentLight.Position = Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform());
		Update();
		PreviousTransform = GetParent()->GetTransform();
	}
}

void PointLightComponent::Destroy()
{
	Graphics::Lights.erase(Graphics::Lights.begin() + GetLightIndex());
}


void PointLightComponent::SetColor(Vector3 NewColor)
{
	CurrentLight.Color = NewColor;
}

Vector3 PointLightComponent::GetColor()
{
	return CurrentLight.Color;
}

void PointLightComponent::SetIntensity(float NewIntensity)
{
	CurrentLight.Intensity = NewIntensity;
}

float PointLightComponent::GetIntensity()
{
	return CurrentLight.Intensity;
}

void PointLightComponent::SetFalloff(float NewFalloff)
{
	CurrentLight.Falloff = NewFalloff;
}

float PointLightComponent::GetFalloff()
{
	return CurrentLight.Falloff;
}

void PointLightComponent::Update()
{
	Graphics::Lights[GetLightIndex()] = CurrentLight;
	PreviousLight = CurrentLight;
}

size_t PointLightComponent::GetLightIndex()
{
	for (size_t i = 0; i < Graphics::Lights.size(); i++)
	{
		if (Graphics::Lights[i] == PreviousLight)
		{
			return i;
		}
	}
	throw("Could not find light index");
}
