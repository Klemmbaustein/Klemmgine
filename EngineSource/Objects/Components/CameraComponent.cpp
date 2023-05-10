#pragma once
#include "CameraComponent.h"
#include <World/Stats.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Log.h>

void CameraComponent::Begin()
{
}

void CameraComponent::Tick()
{
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
	//ComponentCamera.Update();
}

void CameraComponent::Destroy()
{
	for (FramebufferObject* f : Graphics::AllFramebuffers)
	{
		if (f->FramebufferCamera == &ComponentCamera)
		{
			f->FramebufferCamera = nullptr;
		}
	}
}

void CameraComponent::SetTransform(Transform NewTransform)
{
	this->RelativeTransform = NewTransform;
}

Transform& CameraComponent::GetTransform()
{
	return RelativeTransform;
}

void CameraComponent::SetFOV(float FOVinRadiants)
{
	ComponentCamera.ReInit(FOVinRadiants, Graphics::WindowResolution.X, Graphics::WindowResolution.Y, false);
}

CameraComponent::CameraComponent()
{
}

void CameraComponent::Use()
{
	if (!IsInEditor)
	{
		Graphics::MainFramebuffer->FramebufferCamera = &ComponentCamera;
		Graphics::MainCamera = &ComponentCamera;
	}
}