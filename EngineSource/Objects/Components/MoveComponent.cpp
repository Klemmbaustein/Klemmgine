#include "MoveComponent.h"
#include <Engine/Stats.h>
#include <Math/Physics/Physics.h>
#include <Engine/Log.h>
#include <Networking/Client.h>

#define GET_COLLISION_PTR() Physics::CapsuleBody* CollisionBody = static_cast<Physics::CapsuleBody*>(CollisionBodyPtr)

Vector3 MoveComponent::TryMove(Vector3 Direction, Vector3 InitialDireciton, Vector3 Pos, bool GravityPass, int Depth)
{
	GET_COLLISION_PTR();

	float Distance = Direction.Length() + 0.01f;

	auto Hits = CollisionBody->ShapeCast(Transform(Pos, 0, Vector3(ColliderSize.X, ColliderSize.Y, ColliderSize.X)), Pos + Direction.Normalize() * Distance);

	if (Depth >= MoveMaxDepth)
	{
		return 0;
	}

	if (!Hits.size())
	{
		if (GravityPass && GroundedTimer > 0)
		{
			GroundedTimer--;
		}
		else
		{
			GroundNormal = Vector3(0, 1, 0);
		}
		return Direction;
	}

	for (auto& i : Hits)
	{
		float AbsoluteDistance = i.Distance * Direction.Length();
		Vector3 SnapToSurface = Direction.Normalize() * (AbsoluteDistance - 0.01f);
		Vector3 LeftOver = Direction - SnapToSurface;

		float Angle = Vector3::Dot(i.Normal, Vector3(0, 1, 0));
		float Length = LeftOver.Length();
		LeftOver = LeftOver.ProjectToPlane(0, i.Normal);
		LeftOver = LeftOver.Normalize() * Length;
		if (Angle > 0.75f)
		{
			if (GravityPass)
			{
				GroundedTimer = 5;
				GroundNormal = i.Normal;
				return SnapToSurface;
			}
		}
		else
		{
			float Scale = 1 - Vector3::Dot(
				Vector3(i.Normal.X, 0, i.Normal.Z).Normalize(),
				Vector3(-InitialDireciton.X, 0, -InitialDireciton.Z).Normalize()
			);
			LeftOver = LeftOver * Scale;
		}


		return SnapToSurface + TryMove(LeftOver, InitialDireciton, Pos + SnapToSurface, GravityPass, Depth + 1);
	}

	return Direction;
}

bool MoveComponent::GetIsOnGround() const
{
	return GroundedTimer > 0;
}

Vector3 MoveComponent::GetVelocity()
{
	return Vector3(MovementVelocity.X, VerticalVelocity, MovementVelocity.Y);
}

void MoveComponent::Begin()
{
	Physics::PhysicsBody* CollisionBody = new Physics::CapsuleBody(GetWorldTransform().Position,
		0,
		Vector2(1.0f, 2.0f),
		Physics::MotionType::Static,
		Physics::Layer::Static,
		this);

	CollisionBodyPtr = CollisionBody;
}

void MoveComponent::Update()
{
	if (IsInEditor || !Active || (GetParent()->GetIsReplicated() && Client::GetIsConnected() && GetParent()->NetOwner != Client::GetClientID()))
	{
		return;
	}
	GET_COLLISION_PTR();


	Transform WorldTransform = GetWorldTransform();

	InputDirection.Y = 0;
	Vector3 MoveDir = (InputDirection * Performance::DeltaTime * 15).ProjectToPlane(0, GroundNormal);
	GetParent()->GetTransform().Position += TryMove(MoveDir, MoveDir, WorldTransform.Position, false);

	MoveDir = Vector3(0, (std::min(VerticalVelocity, -5.0f) + (Jumping ? JumpHeight : 0)) * Performance::DeltaTime, 0);
	Vector3 GravityMovement = TryMove(MoveDir, MoveDir, WorldTransform.Position, true);

	GetParent()->GetTransform().Position += GravityMovement;

	if (GroundedTimer == 0)
	{
		VerticalVelocity -= Performance::DeltaTime * Gravity;
	}
	else
	{
		VerticalVelocity = 0;
		Jumping = false;
	}
	
	InputDirection = 0;

	CollisionBody->SetTransform(Transform(WorldTransform.Position, 0, Vector3(ColliderSize.X, ColliderSize.Y, ColliderSize.X)));
	auto Hits = CollisionBody->CollisionTest();
	for (auto& h : Hits)
	{
		GetParent()->GetTransform().Position += h.Normal * h.Depth;
	}
}

void MoveComponent::Destroy()
{
}

void MoveComponent::AddMovementInput(Vector3 Direction)
{
	InputDirection += Direction;
}

void MoveComponent::Jump()
{
	if (GetIsOnGround())
	{
		Jumping = true;
		GroundedTimer = 0;
	}
}
