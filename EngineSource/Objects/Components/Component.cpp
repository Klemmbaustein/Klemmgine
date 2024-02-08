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
	Vector3 InvertedRotation = (GetParent()->GetTransform().Rotation + RelativeTransform.Rotation);
	InvertedRotation = Vector3(-InvertedRotation.Z, InvertedRotation.Y, -InvertedRotation.X);
	return Transform(Vector3::TranslateVector(RelativeTransform.Position, GetParent()->GetTransform()),
		Vector3() - InvertedRotation.DegreesToRadians(),
		RelativeTransform.Scale * GetParent()->GetTransform().Scale);
}
