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
	yaw = -90.0f;
	pitch = 0.0f;
	OnMouseMoved(0.0f, 0.0f);
	Update();
}

void Camera::OnMouseMoved(float xRel, float yRel)
{
	yaw += xRel * mouseSensitivity;
	pitch -= yRel * mouseSensitivity;
}

void Camera::UpdateRotation()
{
	Vector3 EffectiveRotation = Vector3(pitch, yaw, roll) + CameraShake::CameraShakeTranslation;

	Vector3 front;
	front.X = cos(glm::radians(EffectiveRotation.X)) * cos(glm::radians(EffectiveRotation.Y));
	front.Y = sin(glm::radians(EffectiveRotation.X));
	front.Z = cos(glm::radians(EffectiveRotation.X)) * sin(glm::radians(EffectiveRotation.Y));

	Up = Vector3::GetUpVector(EffectiveRotation);
	lookAt = front.Normalize();
	Right = Vector3::Cross(front, Up).Normalize();
}

void Camera::Update()
{
	View = glm::lookAt(glm::vec3(0), (glm::vec3)lookAt, (glm::vec3)Up);
	View = glm::rotate(View, glm::radians(roll), (glm::vec3)lookAt);
	View = glm::translate(View, -(glm::vec3)Position);

	ViewProj = Projection * View;
	Rotation = Vector3(pitch, yaw, roll);
	UpdateRotation();
}

glm::mat4 Camera::getViewProj()
{
	return ViewProj;
}

Vector3 Camera::ForwardVectorFromScreenPosition(float x, float y)
{
	glm::vec4 ray_clip = glm::vec4(x, y, -1.f, 1);

	glm::vec4 ray_eye = glm::inverse(Projection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.f, 1.f);

	Vector3 ray_wor = glm::vec3(glm::inverse(View) * ray_eye);

	ray_wor -= Position;
	return ray_wor.Normalize();
}

void Camera::ReInit(float FOV, float Width, float Heigth, bool Ortho)
{
	this->FOV = FOV;
	if (Ortho)
		Projection = glm::ortho(-FOV, FOV, -FOV, FOV, -25.f, 25.f);
	else
		Projection = glm::perspective(FOV / 2, Width / Heigth, NearPlane, FarPlane);
}