#include "PhysicsSubsystem.h"
#include <Math/Physics/Physics.h>
#include <Math/Collision/CollisionVisualize.h>

PhysicsSubsystem* PhysicsSubsystem::PhysicsSystem = nullptr;

PhysicsSubsystem::PhysicsSubsystem()
{
	PhysicsSystem = this;
	Name = "Physics";

	Physics::Init();

}

void PhysicsSubsystem::Update()
{
	Physics::Update();
	CollisionVisualize::Update();
}
