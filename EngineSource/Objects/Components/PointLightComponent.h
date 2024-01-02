#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Graphics.h>

class PointLightComponent : public Component
{
public:
	void Begin() override;
	void Update() override;
	void Destroy() override;


	void SetColor(Vector3 NewColor);
	Vector3 GetColor();

	void SetIntensity(float NewIntensity);
	float GetIntensity();

	void SetFalloff(float NewFalloff);
	float GetFalloff();

protected:
	Graphics::Light CurrentLight;
	Graphics::Light PreviousLight;

	Transform PreviousTransform;

	void UpdateLight();
	size_t GetLightIndex() const;
};