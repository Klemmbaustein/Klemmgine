#pragma once
#include <Objects/Components/MeshComponent.h>
#include <Math/Collision/Collision.h>

/**
* @brief
* Collision mesh Component.
* 
* Contains collision data.
* 
* @ingroup Components
*/
class CollisionComponent : public Component
{
public:
	Collision::HitResponse OverlapCheck(std::set<CollisionComponent*> MeshesToIgnore = {});
	CollisionComponent() {}
	virtual void Destroy() override;
	void Update() override;
	void Load(std::vector<Vertex> Vertices, std::vector<unsigned int> Indices, Transform RelativeTransform = Transform());
	Collision::CollisionMesh* CollMesh = nullptr;
protected:
	Transform LastParentTransform;
	Transform LastRelativeTransform;
};