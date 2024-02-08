#include "PhysicsComponent.h"
#include <Math/Physics/Physics.h>
#include <Engine/Log.h>

void PhysicsComponent::Begin()
{
}

void PhysicsComponent::Destroy()
{
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);

	Physics::RemoveBody(Body);
	delete Body;
}


void PhysicsComponent::CreateBox(Transform RelativeTransform, Physics::MotionType BoxMovability, Physics::Layer CollisionLayers)
{
	if (PhysicsBodyPtr)
	{
		Destroy();
	}

	this->RelativeTransform = RelativeTransform;

	Transform ComponentTransform = Component::GetWorldTransform();

	Physics::PhysicsBody* Body = new Physics::BoxBody(ComponentTransform.Position,
		ComponentTransform.Rotation,
		ComponentTransform.Scale,
		(Physics::MotionType)BoxMovability,
		CollisionLayers,
		this);

	
	PhysicsBodyPtr = Body;
	Physics::AddBody(Body);
}

void PhysicsComponent::CreateSphere(Transform RelativeTransform, Physics::MotionType BoxMovability, Physics::Layer CollisionLayers)
{
	if (PhysicsBodyPtr)
	{
		Destroy();
	}

	this->RelativeTransform = RelativeTransform;

	Transform ComponentTransform = Component::GetWorldTransform();
	
	Physics::PhysicsBody* Body = new Physics::SphereBody(ComponentTransform.Position,
		ComponentTransform.Rotation,
		(ComponentTransform.Scale.X + ComponentTransform.Scale.Y + ComponentTransform.Scale.Z) / 3,
		BoxMovability,
		CollisionLayers,
		this);


	PhysicsBodyPtr = Body;
	Physics::AddBody(Body);
}


Transform PhysicsComponent::GetWorldTransform()
{
	if (!PhysicsBodyPtr)
	{
		return Transform();
	}
	
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	return Transform(Body->GetPosition(), Body->GetRotation(), 1);
}
