#pragma once
#include <Objects/WorldObject.h>

class ComponentSetter;

class Component
{
public:
	Component() {}
	virtual ~Component() {}

	virtual void Begin();
	virtual void Tick();
	virtual void Destroy();
	WorldObject* GetParent();
	friend class ComponentModifier;
private:
	WorldObject* Parent = nullptr;
};

class ComponentModifier
{
public:
	static void SetParent(Component* c, WorldObject* p)
	{
		c->Parent = p;
	}
};