#pragma once
#include <Objects/Components/Component.h>
#include <Math/Physics/Physics.h>

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
	* @parma BoxMovability
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
	* @parma BoxMovability
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
	* @parma BoxMovability
	* The movability of the PhysicsBody.
	* @param CollisionLayers
	* The layers of the PhysicsBody.
	*/
	void CreateCapsule(Transform RelativeTransform, Physics::MotionType CapsuleMovability, Physics::Layer CollisionLayers = Physics::Layer::Dynamic);
	
	/// Gets the transform of the simulated physics body, in world space.
	Transform GetWorldTransform() override;

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

private:
	void* PhysicsBodyPtr = nullptr;
};