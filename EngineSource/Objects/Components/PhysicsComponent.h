#pragma once
#include <Objects/Components/Component.h>
#include <Math/Physics/Physics.h>
#include <set>

/**
* @brief
* A component that simulates physics.
* 
* @ingroup Components
*/
class PhysicsComponent : public Component
{
public:
	void Begin() override;
	virtual void Destroy() override;

	/**
	* @brief
	* Creates a @ref Physics::BoxBody with the given parameters.
	* 
	* @param RelativeTransform
	* The Transform of the PhysicsBody, relative to this component's parent.
	* @param BoxMovability
	* The movability of the PhysicsBody.
	* @param CollisionLayers
	* The layers of the PhysicsBody.
	*/
	void CreateBox(Transform RelativeTransform, Physics::MotionType BoxMovability, Physics::Layer CollisionLayers = Physics::Layer::Dynamic);
	
	/**
	* @brief
	* Creates a @ref Physics::SphereBody with the given parameters.
	*
	* @param RelativeTransform
	* The Transform of the PhysicsBody, relative to this component's parent.
	* @param SphereMovability
	* The movability of the PhysicsBody.
	* @param CollisionLayers
	* The layers of the PhysicsBody.
	*/
	void CreateSphere(Transform RelativeTransform, Physics::MotionType SphereMovability, Physics::Layer CollisionLayers = Physics::Layer::Dynamic);
	
	/**
	* @brief
	* Creates a @ref Physics::CapsuleBody with the given parameters.
	*
	* @param RelativeTransform
	* The Transform of the PhysicsBody, relative to this component's parent.
	* @param CapsuleMovability
	* The movability of the PhysicsBody.
	* @param CollisionLayers
	* The layers of the PhysicsBody.
	*/
	void CreateCapsule(Transform RelativeTransform, Physics::MotionType CapsuleMovability, Physics::Layer CollisionLayers = Physics::Layer::Dynamic);
	
	/// Gets the transform of the simulated physics body, in world space.
	Transform GetBodyWorldTransform();

	/// Sets the position of the physics body, in world space.
	void SetPosition(Vector3 NewPosition);
	/// Sets the rotation of the physics body, in world space.
	void SetRotation(Vector3 NewRotation);
	/// Sets the scale of the physics body, in world space.
	void SetScale(Vector3 NewScale);

	/// Gets the velocity of the physics body.
	Vector3 GetVelocity();
	/// Gets the angular velocity of the physics body.
	Vector3 GetAngularVelocity();

	/// Sets the velocity of the physics body.
	void SetVelocity(Vector3 NewVelocity);
	/// Sets the angular velocity of the physics body.
	void SetAngularVelocity(Vector3 NewVelocity);

	/**
	* @brief
	* Sets the activeness of the body
	* 
	* Active means the collider can interact with the physics system.
	* Inactive means it can't.
	*/
	void SetActive(bool NewActive);

	/**
	* @brief
	* Casts this physics component shape from Start to End.
	* 
	* @param Start
	* The start Transform of the cast.
	* @param End
	* The end position of the cast.
	* @param Layers
	* The layers to check.
	* @param ObjectsToIgnore
	* These objects shouldn't be considered for the query.
	*/
	Physics::HitResult ShapeCast(Transform Start, Vector3 End, Physics::Layer Layers, std::set<WorldObject*> ObjectsToIgnore);

	/**
	* @brief
	* Queries collision with this physics component's shape.
	* 
	* @param Where
	* The Transform of the query.
	* @param Layers
	* The layers to check.
	* @param ObjectsToIgnore
	* These objects shouldn't be considered for the query.
	*/
	Physics::HitResult CollisionCheck(Transform Where, Physics::Layer Layers, std::set<WorldObject*> ObjectsToIgnore);

private:
	bool Active = false;
	void* PhysicsBodyPtr = nullptr;
};