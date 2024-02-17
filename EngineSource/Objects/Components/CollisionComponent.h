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
	CollisionComponent() {}
	virtual void Destroy() override;
	void Update() override;
	void Load(const ModelGenerator::ModelData& Model, Transform RelativeTransform = Transform());
	void* Collider = nullptr;
protected:
	Vector3 LastScale = 0;
	Transform CalculateMeshTransform();
	Transform LastParentTransform;
	Transform LastRelativeTransform;
};