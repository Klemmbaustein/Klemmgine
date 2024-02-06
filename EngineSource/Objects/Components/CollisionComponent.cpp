#include "CollisionComponent.h"
#include <Math/Physics/Physics.h>
#include <Engine/Log.h>


Collision::HitResponse CollisionComponent::OverlapCheck(std::set<CollisionComponent*> MeshesToIgnore)
{
	//CollMesh->SetTransform(Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
	//	RelativeTransform.Rotation + GetParent()->GetTransform().Rotation,
	//	RelativeTransform.Scale * 0.025f * GetParent()->GetTransform().Scale));
	//return CollMesh->OverlapCheck(MeshesToIgnore);
	return Collision::HitResponse();
}

void CollisionComponent::Destroy()
{
	for (int i = 0; i < Collision::CollisionBoxes.size(); i++)
	{
		if (Collision::CollisionBoxes.at(i) == this)
		{
			Collision::CollisionBoxes.erase(Collision::CollisionBoxes.begin() + i);
		}
	}

	Physics::MeshBody* m = static_cast<Physics::MeshBody*>(Collider);

	Physics::RemoveBody(m);

	delete m;
}

void CollisionComponent::Update()
{
	return;
	/*
	if (LastParentTransform != GetParent()->GetTransform() || LastRelativeTransform != RelativeTransform)
	{
		Vector3 InvertedRotation = (GetParent()->GetTransform().Rotation + RelativeTransform.Rotation);
		InvertedRotation = Vector3(-InvertedRotation.Z, InvertedRotation.Y, -InvertedRotation.X);
		CollMesh->SetTransform(Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
			Vector3() - InvertedRotation.DegreesToRadians(),
			RelativeTransform.Scale * 0.025f * GetParent()->GetTransform().Scale));
		LastParentTransform = GetParent()->GetTransform();
		LastRelativeTransform = RelativeTransform;
		
	}*/
}

void CollisionComponent::Load(const ModelGenerator::ModelData& Data, Transform RelativeTransform)
{
	
	this->RelativeTransform = RelativeTransform;
	this->RelativeTransform.Scale = this->RelativeTransform.Scale;
	//CollMesh = new Collision::CollisionMesh(Vertices, Indices, GetParent()->GetTransform() + this->RelativeTransform);
	//Collision::CollisionBoxes.push_back(this);
	auto t = (Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
		RelativeTransform.Rotation + GetParent()->GetTransform().Rotation,
		RelativeTransform.Scale * GetParent()->GetTransform().Scale));

	Physics::MeshBody* m = new Physics::MeshBody(Data, t, Physics::MotionType::Static, Physics::Layer::Static, this);
	Physics::AddBody(m);

	Collider = m;
}