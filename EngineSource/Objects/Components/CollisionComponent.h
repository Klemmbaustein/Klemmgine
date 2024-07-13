#pragma once
#include <Objects/Components/MeshComponent.h>
#include <Math/Collision/Collision.h>

/**
* @brief
* Collision mesh Component.
* 
* Contains collision data, in the form of a 3d mesh.
* 
* @ingroup Components
*/
class CollisionComponent : public Component
{
public:
	CollisionComponent() {}
	virtual void Destroy() override;
	void Update() override;

	/**
	* @brief
	* Loads collision data for this component.
	* 
	* @param Model
	* The model data that should be loaded as collision.
	* 
	* @param RelativeTransform
	* This transform will be applied to the mesh data.
	*/
	void Load(const ModelGenerator::ModelData& Model, Transform RelativeTransform = Transform());


	/**
	* @brief
	* Sets the activeness of the collision mesh.
	*
	* Active means the collider can interact with the physics system.
	* Inactive means it can't.
	*/
	void SetActive(bool NewActive);


	/**
	* @brief
	* Gets the activeness of the collision mesh.
	*
	* Active means the collider can interact with the physics system.
	* Inactive means it can't.
	*/
	bool GetActive() const;

	void* Collider = nullptr;
protected:
	bool Active = true;
	Vector3 LastScale = 0;
	Transform CalculateMeshTransform();
	Transform LastParentTransform;
	Transform LastRelativeTransform;
};