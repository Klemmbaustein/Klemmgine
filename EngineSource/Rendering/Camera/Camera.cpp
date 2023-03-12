#include "Camera.h"
#include <Rendering/Camera/CameraShake.h>
#include <iostream>

Camera::Camera(float FOV, float Width, float Heigth, bool Ortho)
{
	this->FOV = FOV;
	if (Ortho)
		Projection = glm::ortho(-FOV, FOV, -FOV, FOV, -FOV, FOV);
	else
		Projection = glm::perspective(FOV / 2, Width / Heigth, NearPlane, FarPlane);
	View = glm::mat4(1);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
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
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	Vector3 EffectiveRotation = Vector3(pitch, yaw, 0) + CameraShake::CameraShakeTranslation;

	glm::vec3 front;
	front.x = cos(glm::radians(EffectiveRotation.X)) * cos(glm::radians(EffectiveRotation.Y));
	front.y = sin(glm::radians(EffectiveRotation.X));
	front.z = cos(glm::radians(EffectiveRotation.X)) * sin(glm::radians(EffectiveRotation.Y));

	lookAt = glm::normalize(front);
	Right = glm::normalize(glm::cross(front, up));
	Up = glm::normalize(glm::cross(Right, front));
}

void Camera::Update()
{
	View = glm::lookAt(Position, Position + lookAt, up);
	glm::mat4 View = glm::rotate(View, glm::radians(roll), Position + up);

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

	glm::vec3 ray_wor = (glm::inverse(View) * ray_eye);

	ray_wor -= Position;
	return Vector3(ray_wor.x, ray_wor.y, ray_wor.z).Normalize();
}

void Camera::ReInit(float FOV, float Width, float Heigth, bool Ortho)
{
	this->FOV = FOV;
	if (Ortho)
		Projection = glm::ortho(-FOV, FOV, -FOV, FOV, -25.f, 25.f);
	else
		Projection = glm::perspective(FOV / 2, Width / Heigth, NearPlane, FarPlane);
}