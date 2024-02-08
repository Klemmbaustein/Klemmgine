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
	void CreateShape(Physics::PhysicsBody* Body);

	Vector3 GetBodyPosition(Physics::PhysicsBody* Body);
	Vector3 GetBodyRotation(Physics::PhysicsBody* Body);

	void SetBodyPosition(Physics::PhysicsBody* Body, Vector3 NewPosition);
	void SetBodyRotation(Physics::PhysicsBody* Body, Vector3 NewRotation);
	void MultiplyBodyScale(Physics::PhysicsBody* Body, Vector3 Scale);

	void Update();

	std::vector<Physics::HitResult> CollisionTest(Physics::PhysicsBody* Body);
	std::vector<Physics::HitResult> ShapeCastBody(Physics::PhysicsBody* Body, Transform StartPos, Vector3 EndPos);
	Physics::HitResult LineCast(Vector3 Start, Vector3 End);
}