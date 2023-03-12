#pragma once
#include <Objects/WorldObject.h>
#include <GENERATED/GENERATED_LightObject.h>

class PointLightComponent;

class LightObject : public WorldObject
{
public:
	LIGHTOBJECT_GENERATED("Default/Misc")
	void Begin() override;
	void OnPropertySet() override;
	PointLightComponent* Light = nullptr;

	Vector3 Color = Vector3(1);
	float Intensity = 2.0;
	float Falloff = 1.0;
};