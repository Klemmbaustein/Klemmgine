#include "PhysicsComponent.h"
#include <Math/Physics/Physics.h>
#include <iostream>

void PhysicsComponent::Begin()
{
}

void PhysicsComponent::Destroy()
{
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	if (Body)
	{
		SetActive(true);
		Physics::RemoveBody(Body, true);
		delete Body;
	}
}

void PhysicsComponent::Update()
{
	if (!PhysicsBodyPtr)
	{
		return;
	}

	if (!GetParent())
	{
		return;
	}

	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);

	if (Body->ColliderMovability == Physics::MotionType::Static && Active)
	{
		Transform ComponentTransform = Component::GetWorldTransform();
		Body->SetPosition(ComponentTransform.Position);
		Body->SetRotation(ComponentTransform.Rotation);
	}
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
	SetActive(true);
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
	SetActive(true);
}

void PhysicsComponent::CreateCapsule(Transform RelativeTransform, Physics::MotionType CapsuleMovability, Physics::Layer CollisionLayers)
{
	if (PhysicsBodyPtr)
	{
		Destroy();
	}

	this->RelativeTransform = RelativeTransform;

	Transform ComponentTransform = GetWorldTransform();

	Physics::PhysicsBody* Body = new Physics::CapsuleBody(ComponentTransform.Position,
		ComponentTransform.Rotation,
		Vector2(ComponentTransform.Scale.X, ComponentTransform.Scale.Y),
		CapsuleMovability,
		CollisionLayers,
		this);


	PhysicsBodyPtr = Body;
	SetActive(true);
}

Transform PhysicsComponent::GetBodyWorldTransform() const
{
	if (!PhysicsBodyPtr)
	{
		return Transform();
	}
	
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	return Transform(Body->GetPosition(), Body->GetRotation(), 1);
}

void PhysicsComponent::SetPosition(Vector3 NewPosition)
{
	if (!PhysicsBodyPtr)
	{
		return;
	}

	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	Body->SetPosition(NewPosition);
}

void PhysicsComponent::SetRotation(Vector3 NewRotation)
{
	if (!PhysicsBodyPtr)
	{
		return;
	}

	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	Body->SetRotation(NewRotation);
}

void PhysicsComponent::SetScale(Vector3 NewScale)
{
	if (!PhysicsBodyPtr)
	{
		return;
	}

	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);

	Vector3 ScaleDifference = NewScale / Body->BodyTransform.Scale;
	if (ScaleDifference != Vector3(1))
	{
		Body->Scale(ScaleDifference);
	}

}

Vector3 PhysicsComponent::GetVelocity() const
{
	if (!PhysicsBodyPtr)
	{
		return 0;
	}
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	return Body->GetVelocity();
}

Vector3 PhysicsComponent::GetAngularVelocity() const
{
	if (!PhysicsBodyPtr)
	{
		return 0;
	}
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	return Body->GetAngularVelocity();
}

void PhysicsComponent::SetVelocity(Vector3 NewVelocity)
{
	if (!PhysicsBodyPtr)
	{
		return;
	}
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	Body->SetVelocity(NewVelocity);
}

void PhysicsComponent::SetAngularVelocity(Vector3 NewVelocity)
{
	if (!PhysicsBodyPtr)
	{
		return;
	}
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	Body->SetAngularVelocity(NewVelocity);
}

void PhysicsComponent::SetActive(bool NewActive)
{
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	if (NewActive == Active || !Body)
	{
		return;
	}
	if (NewActive)
	{
		Physics::AddBody(Body);
	}
	else
	{
		Physics::RemoveBody(Body, false);
	}
	Active = NewActive;
}

Physics::HitResult PhysicsComponent::ShapeCast(Transform Start, Vector3 End, Physics::Layer Layers, std::set<WorldObject*> ObjectsToIgnore)
{
	if (!PhysicsBodyPtr)
	{
		return Physics::HitResult();
	}
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	return Physics::HitResult::GetAverageHit(Body->ShapeCast(Start, End, Layers, ObjectsToIgnore));

}

Physics::HitResult PhysicsComponent::CollisionCheck(Transform Start, Physics::Layer Layers, std::set<WorldObject*> ObjectsToIgnore)
{
	if (!PhysicsBodyPtr)
	{
		return Physics::HitResult();
	}
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(PhysicsBodyPtr);
	Transform InitialTransform = Body->BodyTransform;
	Body->BodyTransform = Start;
	auto Hit =  Physics::HitResult::GetAverageHit(Body->CollisionTest(Layers, ObjectsToIgnore));

	std::swap(InitialTransform, Body->BodyTransform);

	return Hit;
}
