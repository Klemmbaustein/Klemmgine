#pragma once
#include <Objects/SceneObject.h>
#include <GENERATED/LightObject.h>

class PointLightComponent;
class BillboardComponent;

/**
* @brief
* An object representing a point light in a scene.
* 
* @ingroup Objects
* 
* Path: Classes/Default
*/
class LightObject : public SceneObject
{
public:
	LIGHTOBJECT_GENERATED("Default")
	void Begin() override;
	void OnPropertySet() override;
	PointLightComponent* Light = nullptr;

	/**
	* @brief 
	* An editor parameter. The color of the light.
	*/
	Vector3 Color = Vector3(1);
	
	/**
	* @brief 
	* An editor parameter. The intensity of the light.
	*/
	float Intensity = 1.0;

	/**
	* @brief 
	* An editor parameter. The falloff of the light.
	*/
	float Falloff = 10.0;
#if EDITOR
	BillboardComponent* Billboard = nullptr;
#endif
};