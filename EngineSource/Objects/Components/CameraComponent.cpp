#include "CameraComponent.h"
#include <Engine/Stats.h>
#include <Rendering/Framebuffer.h>
#include <Engine/Log.h>

void CameraComponent::Begin()
{
}

void CameraComponent::Update()
{
#if !SERVER
	Vector3 ParentRotation = GetParent()->GetTransform().Rotation;
	ParentRotation = Vector3(ParentRotation.X, ParentRotation.Y, 0);
	Transform ParentTransform = Transform(GetParent()->GetTransform().Position,
		Vector3(0, ParentRotation.Y, -ParentRotation.X).DegreesToRadians(), GetParent()->GetTransform().Scale * RelativeTransform.Scale);
	auto Position = Vector3::TranslateVector(RelativeTransform.Position, ParentTransform);
	ComponentCamera.Position = Position;
	ParentRotation = GetParent()->GetTransform().Rotation + RelativeTransform.Rotation;
	ParentRotation = Vector3(ParentRotation.X, ParentRotation.Y, 0);
	ComponentCamera.SetRotation(ParentRotation);
	ComponentCamera.Update();
#endif
}

void CameraComponent::Destroy()
{
#if !SERVER
	for (FramebufferObject* f : Graphics::AllFramebuffers)
	{
		if (f->FramebufferCamera == &ComponentCamera)
		{
			f->FramebufferCamera = nullptr;
		}
	}
#endif
}


void CameraComponent::SetFOV(float FOV)
{
#if !SERVER
	ComponentCamera.ReInit(glm::radians(FOV) * 2, Graphics::WindowResolution.X, Graphics::WindowResolution.Y, false);
#endif
}

float CameraComponent::GetFOV() const
{
	return glm::degrees(ComponentCamera.FOV) / 2;
}

CameraComponent::CameraComponent()
{
}

void CameraComponent::Use()
{
#if !SERVER
	if (!IsInEditor)
	{
		Graphics::MainFramebuffer->FramebufferCamera = &ComponentCamera;
		Graphics::MainCamera = &ComponentCamera;
	}
#endif
}