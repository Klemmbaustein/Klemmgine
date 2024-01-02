#include <Objects/Components/Component.h>

void Component::Begin()
{
}

void Component::Update()
{
}

void Component::Destroy()
{
}


WorldObject* Component::GetParent()
{
	return Parent;
}