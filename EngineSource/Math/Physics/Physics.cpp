#include "Physics.h"
#include "JoltPhysics.h"
#include <Engine/EngineError.h>

void Physics::Init()
{
	JoltPhysics::Init();
}

void Physics::Update()
{
	JoltPhysics::Update();
}

void Physics::AddBody(PhysicsBody* Body)
{
	JoltPhysics::RegisterBody(Body);
}

void Physics::RemoveBody(PhysicsBody* Body)
{
	JoltPhysics::RemoveBody(Body);
}

Physics::HitResult Physics::RayCast(Vector3 Start, Vector3 End)
{
	return JoltPhysics::LineCast(Start, End);
}

Physics::PhysicsBody::PhysicsBody(BodyType Type, Transform BodyTransform, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
{
	this->Type = Type;
	this->BodyTransform = BodyTransform;
	this->ColliderMovability = ColliderMovability;
	this->CollisionLayers = CollisionLayers;
	this->Parent = Parent;
}

const Transform& Physics::PhysicsBody::GetTransform() const
{
	return BodyTransform;
}

Vector3 Physics::PhysicsBody::GetPosition()
{
	return JoltPhysics::GetBodyPosition(this);
}

Vector3 Physics::PhysicsBody::GetRotation()
{
	return JoltPhysics::GetBodyRotation(this);
}

Physics::SphereBody::SphereBody(Vector3 Position, Vector3 Rotation, float Scale, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Sphere, Transform(Position, Rotation, Scale), ColliderMovability, CollisionLayers, Parent)
{
}

void Physics::SphereBody::SetSphereTransform(Vector3 Position, Vector3 Rotation, float Scale)
{
	this->BodyTransform = Transform(Position, Rotation, Scale);
}

void Physics::SphereBody::SetTransform(Transform T)
{
	ENGINE_UNREACHABLE();
}

Physics::MeshBody::MeshBody(const ModelGenerator::ModelData& Mesh, Transform MeshTransform, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Mesh, MeshTransform, ColliderMovability, CollisionLayers, Parent)
{
	this->MeshData = Mesh;
}

void Physics::MeshBody::SetTransform(Transform T)
{
	this->BodyTransform = T;
}
