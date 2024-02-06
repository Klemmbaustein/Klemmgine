#pragma once
#include "Physics.h"

/**
* @brief
* Namespace containing the jolt physics implementation.
*/
namespace JoltPhysics
{
	void Init();

	void RegisterBody(Physics::PhysicsBody* Body);
	void RemoveBody(Physics::PhysicsBody* Body);

	Vector3 GetBodyPosition(Physics::PhysicsBody* Body);

	void Update();

	Physics::HitResult LineCast(Vector3 Start, Vector3 End);
}