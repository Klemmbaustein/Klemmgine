#include "Physics.h"
#include "JoltPhysics.h"
#include <Engine/EngineError.h>
#include <Objects/WorldObject.h>

#define PHYSICS_SYSTEM JoltPhysics

void Physics::Init()
{
	PHYSICS_SYSTEM::Init();
}

void Physics::Update()
{
	PHYSICS_SYSTEM::Update();
}

void Physics::AddBody(PhysicsBody* Body)
{
	PHYSICS_SYSTEM::RegisterBody(Body);
}

void Physics::RemoveBody(PhysicsBody* Body)
{
	PHYSICS_SYSTEM::RemoveBody(Body);
}

Physics::HitResult Physics::RayCast(Vector3 Start, Vector3 End, Layer Layers, std::set<WorldObject*> ObjectsToIgnore)
{
	return PHYSICS_SYSTEM::LineCast(Start, End, Layers, ObjectsToIgnore);
}

Physics::PhysicsBody::PhysicsBody(BodyType NativeType, Transform BodyTransform, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
{
	this->NativeType = NativeType;
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
	return PHYSICS_SYSTEM::GetBodyPosition(this);
}

Vector3 Physics::PhysicsBody::GetRotation()
{
	return PHYSICS_SYSTEM::GetBodyRotation(this);
}

void Physics::PhysicsBody::SetPosition(Vector3 NewPosition)
{
	PHYSICS_SYSTEM::SetBodyPosition(this, NewPosition);
}

void Physics::PhysicsBody::SetRotation(Vector3 NewRotation)
{
	PHYSICS_SYSTEM::SetBodyRotation(this, NewRotation);
}

void Physics::PhysicsBody::Scale(Vector3 ScaleMultiplier)
{
	PHYSICS_SYSTEM::MultiplyBodyScale(this, ScaleMultiplier);
}

void Physics::PhysicsBody::AddForce(Vector3 Direction, Vector3 Point)
{
	PHYSICS_SYSTEM::AddBodyForce(this, Direction, Point);
}

void Physics::PhysicsBody::SetVelocity(Vector3 NewVelocity)
{
	PHYSICS_SYSTEM::SetBodyVelocity(this, NewVelocity);
}

void Physics::PhysicsBody::SetAngularVelocity(Vector3 NewVelocity)
{
	PHYSICS_SYSTEM::SetBodyAngularVelocity(this, NewVelocity);
}

Vector3 Physics::PhysicsBody::GetVelocity()
{
	return PHYSICS_SYSTEM::GetBodyVelocity(this);
}

Vector3 Physics::PhysicsBody::GetAngularVelocity()
{
	return PHYSICS_SYSTEM::GetBodyAngularVelocity(this);
}

std::vector<Physics::HitResult> Physics::PhysicsBody::CollisionTest()
{
	return PHYSICS_SYSTEM::CollisionTest(this);
}

std::vector<Physics::HitResult> Physics::PhysicsBody::ShapeCast(Transform StartTransform, Vector3 EndPos)
{
	return PHYSICS_SYSTEM::ShapeCastBody(this, StartTransform, EndPos);
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

Physics::BoxBody::BoxBody(Vector3 Position, Vector3 Rotation, Vector3 Extents, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Box, Transform(Position, Rotation, Extents), ColliderMovability, CollisionLayers, Parent)
{
}

void Physics::BoxBody::SetTransform(Transform T)
{
	this->BodyTransform = T;
}

Physics::CapsuleBody::CapsuleBody(Vector3 Position, Vector3 Rotation, Vector2 Scale, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Capsule, Transform(Position, Rotation, Vector3(Scale.X, Scale.Y, Scale.X)), ColliderMovability, CollisionLayers, Parent)
{
}

void Physics::CapsuleBody::SetTransform(Transform T)
{
	this->BodyTransform = T;
}
