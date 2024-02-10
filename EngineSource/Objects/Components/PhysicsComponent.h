#pragma once
#include <Objects/Components/Component.h>
#include <Math/Physics/Physics.h>

class PhysicsComponent : public Component
{
public:
	void Begin() override;
	virtual void Destroy() override;

	void CreateBox(Transform RelativeTransform, Physics::MotionType BoxMovability, Physics::Layer CollisionLayers = Physics::Layer::Dynamic);
	void CreateSphere(Transform RelativeTransform, Physics::MotionType SphereMovability, Physics::Layer CollisionLayers = Physics::Layer::Dynamic);
	void CreateCapsule(Transform RelativeTransform, Physics::MotionType CapsuleMovability, Physics::Layer CollisionLayers = Physics::Layer::Dynamic);
	Transform GetWorldTransform() override;

	void SetPosition(Vector3 NewPosition);
	void SetRotation(Vector3 NewRotation);
	void SetScale(Vector3 NewScale);

private:
	void* PhysicsBodyPtr = nullptr;
};