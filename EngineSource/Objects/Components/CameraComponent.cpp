#include "CameraComponent.h"
#include <Engine/Stats.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Log.h>

void CameraComponent::Begin()
{
}

void CameraComponent::Tick()
{
#if !SERVER
	Vector3 ParentRotation = GetParent()->GetTransform().Rotation;
	ParentRotation = Vector3(ParentRotation.X, ParentRotation.Y, 0);
	Transform ParentTransform = Transform(GetParent()->GetTransform().Location,
		Vector3(0, ParentRotation.Y, -ParentRotation.X).DegreesToRadiants(), GetParent()->GetTransform().Scale * RelativeTransform.Scale);
	auto Location = Vector3::TranslateVector(RelativeTransform.Location, ParentTransform);
	ComponentCamera.Position = Location;
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
	ComponentCamera.ReInit((FOV / 180) * 3.14159f * 2, Graphics::WindowResolution.X, Graphics::WindowResolution.Y, false);
#endif
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