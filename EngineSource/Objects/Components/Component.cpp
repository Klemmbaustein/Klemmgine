#include <Objects/Components/Component.h>

void Component::Begin()
{
}

void Component::Tick()
{
}

void Component::Destroy()
{
}


WorldObject* Component::GetParent()
{
	return Parent;
}