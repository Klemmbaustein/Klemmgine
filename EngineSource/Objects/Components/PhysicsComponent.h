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
	Transform GetWorldTransform() override;
private:
	void* PhysicsBodyPtr = nullptr;
};