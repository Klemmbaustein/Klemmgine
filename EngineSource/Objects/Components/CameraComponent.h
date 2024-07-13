#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Graphics.h>
#include <Rendering/Camera/Camera.h>

/**
* @brief
* A camera.
* 
* When used, the game view will be shown from this component's position.
* 
* @ingroup Components
*/
class CameraComponent : public Component
{
public:
	virtual void Begin() override;
	virtual void Update() override;
	virtual void Destroy() override;

	/// Sets the field of view of the camera in degrees.
	void SetFOV(float FOV);
	/// Gets the field of view of the camera in degrees.
	float GetFOV() const;
	CameraComponent();

	void Use();
protected:
	Camera ComponentCamera = Camera(2, Graphics::WindowResolution.X, Graphics::WindowResolution.Y, false);
};