#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Graphics.h>

/**
* @brief
* A point light component.
* 
* @ingroup Components
*/
class PointLightComponent : public Component
{
public:
	void Begin() override;
	void Update() override;
	void Destroy() override;

	/// Sets the color of the light.
	void SetColor(Vector3 NewColor);
	/// Gets the color of the light.
	Vector3 GetColor();

	/// Sets the light's intensity. Intensity controls brightness.
	void SetIntensity(float NewIntensity);
	/// Gets the light's intensity.
	float GetIntensity();

	/// Sets the falloff of the light. A higher falloff means the light has a higher range.
	void SetFalloff(float NewFalloff);
	/// Gets the falloff of the light.
	float GetFalloff();

protected:
	Graphics::Light CurrentLight;
	Graphics::Light PreviousLight;

	Transform PreviousTransform;

	void UpdateLight();
	size_t GetLightIndex() const;
};