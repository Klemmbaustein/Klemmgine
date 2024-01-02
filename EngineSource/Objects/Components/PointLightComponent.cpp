#include "PointLightComponent.h"
#include <Engine/Log.h>
#include <Rendering/Utility/Framebuffer.h>

void PointLightComponent::Begin()
{
#if !SERVER
	Graphics::MainFramebuffer->Lights.push_back(CurrentLight);
#endif
}

void PointLightComponent::Update()
{
#if !SERVER
	if (CurrentLight != PreviousLight || PreviousTransform != GetParent()->GetTransform())
	{
		CurrentLight.Position = Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform());
		UpdateLight();
		PreviousTransform = GetParent()->GetTransform();
	}
#endif
}

void PointLightComponent::Destroy()
{
#if !SERVER
	Graphics::MainFramebuffer->Lights.erase(Graphics::MainFramebuffer->Lights.begin() + GetLightIndex());
#endif
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

void PointLightComponent::UpdateLight()
{
#if !SERVER
	Graphics::MainFramebuffer->Lights[GetLightIndex()] = CurrentLight;
	PreviousLight = CurrentLight;
#endif
}

size_t PointLightComponent::GetLightIndex() const
{
#if !SERVER
	for (size_t i = 0; i < Graphics::MainFramebuffer->Lights.size(); i++)
	{
		if (Graphics::MainFramebuffer->Lights[i] == PreviousLight)
		{
			return i;
		}
	}
#endif
	return 0;
}
