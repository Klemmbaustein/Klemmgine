#include "PhysicsSubsystem.h"
#include <Math/Physics/Physics.h>
#include <Math/Collision/CollisionVisualize.h>
#include "Console.h"

PhysicsSubsystem* PhysicsSubsystem::PhysicsSystem = nullptr;

PhysicsSubsystem::PhysicsSubsystem()
{
	PhysicsSystem = this;
	Name = "Physics";

	Physics::Init();

#if !SERVER
	Console::ConsoleSystem->RegisterCommand(Console::Command("show_collision", CollisionVisualize::Activate, {}));
	Console::ConsoleSystem->RegisterCommand(Console::Command("hide_collision", CollisionVisualize::Deactivate, {}));
#endif
}

void PhysicsSubsystem::Update()
{
	Physics::Update();
#if !SERVER
	CollisionVisualize::Update();
#endif
}
