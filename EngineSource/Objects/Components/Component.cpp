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

Transform Component::GetWorldTransform()
{
	Vector3 Rot = (GetParent()->GetTransform().Rotation + RelativeTransform.Rotation);
	return Transform(Vector3::TranslateVector(RelativeTransform.Position, GetParent()->GetTransform(), true),
		Rot.DegreesToRadians(),
		RelativeTransform.Scale * GetParent()->GetTransform().Scale);
}
