#include "CollisionComponent.h"
#include <Math/Collision/Collision.h>
#include <Engine/Log.h>


Collision::HitResponse CollisionComponent::OverlapCheck(std::set<CollisionComponent*> MeshesToIgnore)
{
	CollMesh->SetTransform(Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
		RelativeTransform.Rotation + GetParent()->GetTransform().Rotation,
		RelativeTransform.Scale * 0.025f * GetParent()->GetTransform().Scale));
	return CollMesh->OverlapCheck(MeshesToIgnore);
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
	delete CollMesh;
}

void CollisionComponent::Tick()
{
	if (LastParentTransform != GetParent()->GetTransform() || LastRelativeTransform != RelativeTransform)
	{
		Vector3 InvertedRotation = (GetParent()->GetTransform().Rotation + RelativeTransform.Rotation);
		InvertedRotation = Vector3(-InvertedRotation.Z, InvertedRotation.Y, -InvertedRotation.X);
		CollMesh->SetTransform(Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
			Vector3() - InvertedRotation.DegreesToRadiants(),
			RelativeTransform.Scale * 0.025f * GetParent()->GetTransform().Scale));
		LastParentTransform = GetParent()->GetTransform();
		LastRelativeTransform = RelativeTransform;
		
	}
}

void CollisionComponent::Init(std::vector<Vertex> Vertices, std::vector<unsigned int> Indices, Transform RelativeTransform)
{
	
	this->RelativeTransform = RelativeTransform;
	this->RelativeTransform.Scale = this->RelativeTransform.Scale;
	CollMesh = new Collision::CollisionMesh(Vertices, Indices, GetParent()->GetTransform() + this->RelativeTransform);
	Collision::CollisionBoxes.push_back(this);
	CollMesh->SetTransform(Transform(Vector3::TranslateVector(RelativeTransform.Location, GetParent()->GetTransform()),
		RelativeTransform.Rotation + GetParent()->GetTransform().Rotation,
		RelativeTransform.Scale * 0.025f * GetParent()->GetTransform().Scale));

}