#pragma once
#include <Objects/Components/Component.h>
#include <Rendering/Graphics.h>
#include <Rendering/Camera/Camera.h>

class CameraComponent : public Component
{
public:
	virtual void Begin() override;
	virtual void Update() override;
	virtual void Destroy() override;

	void SetFOV(float FOV);
	CameraComponent();

	// Will only work in-game. In the editor, this does nothing.
	void Use();
protected:
	Camera ComponentCamera = Camera(2, Graphics::WindowResolution.X, Graphics::WindowResolution.Y, false);
};