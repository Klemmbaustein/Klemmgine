#pragma once
#include "Subsystem.h"

class PhysicsSubsystem : public Subsystem
{
public:

	static PhysicsSubsystem* PhysicsSystem;

	PhysicsSubsystem();

	void Update() override;
};