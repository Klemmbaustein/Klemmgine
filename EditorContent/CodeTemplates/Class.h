#pragma once
#include <Objects/WorldObject.h>
#include <GENERATED/Class.h>

class Class : public WorldObject
{
public:
	CLASS_GENERATED("");

protected:
	void Begin() override;
	void Tick() override;
	void Destroy() override;
};