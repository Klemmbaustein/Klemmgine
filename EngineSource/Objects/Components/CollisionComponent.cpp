#include "CollisionComponent.h"
#include <Math/Physics/Physics.h>
#include <Engine/Log.h>

void CollisionComponent::Destroy()
{
	Physics::MeshBody* m = static_cast<Physics::MeshBody*>(Collider);

	Collider = nullptr;

	Physics::RemoveBody(m, true);

	delete m;
}

void CollisionComponent::Update()
{
	if (LastParentTransform != GetParent()->GetTransform() || LastRelativeTransform != RelativeTransform)
	{
		Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(Collider);

		Transform MeshTransform = CalculateMeshTransform();

		Body->SetPosition(MeshTransform.Position);
		Body->SetRotation(MeshTransform.Rotation);

		Vector3 ScaleDifference = MeshTransform.Scale / LastScale;
		LastScale = MeshTransform.Scale;

		if (ScaleDifference != Vector3(1))
		{
			Body->Scale(ScaleDifference);
		}

		LastParentTransform = GetParent()->GetTransform();
		LastRelativeTransform = RelativeTransform;
		
	}
}

void CollisionComponent::Load(const ModelGenerator::ModelData& Data, Transform RelativeTransform)
{
	this->RelativeTransform = RelativeTransform;
	this->RelativeTransform.Scale = this->RelativeTransform.Scale;
	Transform MeshTransform = CalculateMeshTransform();

	Physics::MeshBody* m = new Physics::MeshBody(Data, CalculateMeshTransform(), Physics::MotionType::Static, Physics::Layer::Static, this);
	Physics::AddBody(m);
	LastScale = MeshTransform.Scale;

	Collider = m;
}

void CollisionComponent::SetActive(bool NewActive)
{
	Physics::PhysicsBody* Body = static_cast<Physics::PhysicsBody*>(Collider);
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

bool CollisionComponent::GetActive() const
{
	return Active;
}

Transform CollisionComponent::CalculateMeshTransform()
{
	return Transform(Vector3::TranslateVector(RelativeTransform.Position, GetParent()->GetTransform()),
		RelativeTransform.Rotation + GetParent()->GetTransform().Rotation,
		RelativeTransform.Scale * GetParent()->GetTransform().Scale);
}
