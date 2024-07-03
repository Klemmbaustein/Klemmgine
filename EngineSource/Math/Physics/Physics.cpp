#include "Physics.h"
#include "JoltPhysics.h"
#include <Engine/EngineError.h>
#include <Objects/SceneObject.h>
#include <iostream>
#include <Math/Collision/CollisionVisualize.h>

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

void Physics::RemoveBody(PhysicsBody* Body, bool Destroy)
{
	PHYSICS_SYSTEM::RemoveBody(Body, Destroy);
#if !SERVER
	CollisionVisualize::OnBodyRemoved();
#endif
}

Physics::HitResult Physics::RayCast(Vector3 Start, Vector3 End, Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	return PHYSICS_SYSTEM::LineCast(Start, End, Layers, ObjectsToIgnore);
}

Physics::PhysicsBody::PhysicsBody(BodyType NativeType, Transform BodyTransform, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
{
	this->Type = NativeType;
	this->BodyTransform = BodyTransform;
	this->ColliderMovability = ColliderMovability;
	this->CollisionLayers = CollisionLayers;
	this->Parent = Parent;
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
	BodyTransform.Scale = BodyTransform.Scale * ScaleMultiplier;
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

std::vector<Physics::HitResult> Physics::PhysicsBody::CollisionTest(Physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	return PHYSICS_SYSTEM::CollisionTest(this, Layers, ObjectsToIgnore);
}

std::vector<Physics::HitResult> Physics::PhysicsBody::ShapeCast(Transform StartTransform, Vector3 EndPos, Physics::Layer Layers, std::set<SceneObject*> ObjectsToIgnore)
{
	return PHYSICS_SYSTEM::ShapeCastBody(this, StartTransform, EndPos, Layers, ObjectsToIgnore);
}

Physics::SphereBody::SphereBody(Vector3 Position, Vector3 Rotation, float Scale, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Sphere, Transform(Position, Rotation, Scale), ColliderMovability, CollisionLayers, Parent)
{
}

Physics::MeshBody::MeshBody(const ModelGenerator::ModelData& Mesh, Transform MeshTransform, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Mesh, MeshTransform, ColliderMovability, CollisionLayers, Parent)
{
	this->MeshData = Mesh;
}

Physics::BoxBody::BoxBody(Vector3 Position, Vector3 Rotation, Vector3 Extents, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Box, Transform(Position, Rotation, Extents), ColliderMovability, CollisionLayers, Parent)
{
}

Physics::CapsuleBody::CapsuleBody(Vector3 Position, Vector3 Rotation, Vector2 Scale, MotionType ColliderMovability, Layer CollisionLayers, Component* Parent)
	: PhysicsBody(BodyType::Capsule, Transform(Position, Rotation, Vector3(Scale.X, Scale.Y, Scale.X)), ColliderMovability, CollisionLayers, Parent)
{
}

Physics::HitResult Physics::HitResult::GetAverageHit(std::vector<HitResult> Hits)
{
	if (!Hits.size())
	{
		return Physics::HitResult();
	}

	Vector3 HitNormal = Vector3(0, 0, 0);
	float MaxDistance = 0;

	float AvgDepth = 0, AvgDist = 0;

	Vector3 AvgPos = 0;

	for (auto& i : Hits)
	{
		HitNormal += i.Normal * i.Depth;
		MaxDistance = std::max(i.Distance, MaxDistance);
		AvgPos += i.ImpactPoint;
		AvgDepth += i.Depth;
		AvgDist += i.Distance;
	}
	HitNormal = HitNormal.Normalize();
	AvgPos = AvgPos / Vector3((float)Hits.size());

	if (HitNormal.Length() == 0)
	{
		HitNormal = 0;
		for (auto& i : Hits)
		{
			HitNormal += i.Normal;
		}
		HitNormal = HitNormal.Normalize();
	}

	Physics::HitResult h;
	h.Normal = HitNormal;
	h.Distance = AvgDist / (float)Hits.size();
	h.Depth = AvgDepth / (float)Hits.size();
	h.HitComponent = Hits[0].HitComponent;
	h.Hit = true;
	return h;
}
