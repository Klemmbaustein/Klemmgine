#include "Camera.h"
#include <Rendering/Camera/CameraShake.h>
#include <iostream>

Camera::Camera(float FOV, float Width, float Height, bool Ortho)
{
	this->FOV = FOV;
	if (Ortho)
		Projection = glm::ortho(-FOV, FOV, -FOV, FOV, -FOV, FOV);
	else
		Projection = glm::perspective(FOV / 2, Width / Height, NearPlane, FarPlane);
	View = glm::mat4(1);
	Yaw = -90.0f;
	Pitch = 0.0f;
	OnMouseMoved(0.0f, 0.0f);
	Update();
}

void Camera::OnMouseMoved(float xRel, float yRel)
{
	Yaw += xRel * mouseSensitivity;
	Pitch -= yRel * mouseSensitivity;
}

void Camera::UpdateRotation()
{
	Vector3 EffectiveRotation = Vector3(Pitch, Yaw, Roll) + CameraShake::CameraShakeTranslation;

	Vector3 front;
	front.X = cos(glm::radians(EffectiveRotation.X)) * cos(glm::radians(EffectiveRotation.Y));
	front.Y = sin(glm::radians(EffectiveRotation.X));
	front.Z = cos(glm::radians(EffectiveRotation.X)) * sin(glm::radians(EffectiveRotation.Y));

	Up = Vector3::GetUpVector(EffectiveRotation);
	LookAt = front.Normalize();
	Right = Vector3::Cross(front, Up).Normalize();
}

void Camera::Update()
{
	View = glm::lookAt(glm::vec3(0), (glm::vec3)LookAt, (glm::vec3)Up);
	View = glm::rotate(View, glm::radians(Roll), (glm::vec3)LookAt);
	View = glm::translate(View, -(glm::vec3)Position);

	ViewProj = Projection * View;
	Rotation = Vector3(Pitch, Yaw, Roll);
	UpdateRotation();
}

glm::mat4 Camera::getViewProj()
{
	return ViewProj;
}

Vector3 Camera::ForwardVectorFromScreenPosition(float x, float y) const
{
	glm::vec4 RayClip = glm::vec4(x, y, -1.f, 1);

	glm::vec4 RayEye = glm::inverse(Projection) * RayClip;
	RayEye = glm::vec4(RayEye.x, RayEye.y, -1.f, 1.f);

	Vector3 RayWorld = glm::vec3(glm::inverse(View) * RayEye);

	RayWorld -= Position;
	return RayWorld.Normalize();
}

void Camera::ReInit(float FOV, float Width, float Height, bool Ortho)
{
	this->FOV = FOV;
	if (Ortho)
		Projection = glm::ortho(-FOV, FOV, -FOV, FOV, -25.f, 25.f);
	else
		Projection = glm::perspective(FOV / 2, Width / Height, NearPlane, FarPlane);
}