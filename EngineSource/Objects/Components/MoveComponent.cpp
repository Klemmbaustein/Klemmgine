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

	Vector3 HitNormal = Vector3(0, 0, 0);
	float MaxDistance = 0;

	bool HitStep = false;

	Vector3 AvgPos = 0;

	for (auto& i : Hits)
	{
		HitNormal += i.Normal * i.Depth;
		MaxDistance = std::max(i.Distance, MaxDistance);
		AvgPos += i.ImpactPoint;
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

	float Angle = Vector3::Dot(HitNormal, Vector3(0, 1, 0));

	if (!GravityPass)
	{
		Vector3 NewDir = (-HitNormal * Vector3(1, 0, 1)).Normalize() * 1;
		Vector3 TestPos = Pos + Vector3(0, 0.9f, 0);
		auto Hits = CollisionBody->ShapeCast(Transform(TestPos, 0, Vector3(ColliderSize.X, ColliderSize.Y, ColliderSize.X)), TestPos + NewDir);
		HitStep = AvgPos.Y - Pos.Y < -0.9f && !Hits.size();
		if (HitStep)
		{
			HitNormal = Vector3(0, 1, 0);
		}
	}

	float AbsoluteDistance = MaxDistance * Direction.Length();
	Vector3 SnapToSurface = Direction.Normalize() * (AbsoluteDistance - 0.01f);
	Vector3 LeftOver = Direction - SnapToSurface;

	float Length = LeftOver.Length();
	LeftOver = LeftOver.ProjectToPlane(0, HitNormal);
	LeftOver = LeftOver.Normalize() * Length;
	if (Angle > 0.75f)
	{
		if (GravityPass)
		{
			GroundedTimer = 5;
			GroundNormal = HitNormal;
			return SnapToSurface;
		}
	}
	else
	{
		if (Angle < -0.5)
		{
			Jumping = false;
			VerticalVelocity = 0;
		}

		float Scale = 1 - Vector3::Dot(
		Vector3(HitNormal.X, 0, HitNormal.Z).Normalize(),
			Vector3(-InitialDireciton.X, 0, -InitialDireciton.Z).Normalize()
		);
		LeftOver = LeftOver * Scale;
	}

	if (!GravityPass)
	{
		SnapToSurface += HitNormal * Performance::DeltaTime * (HitStep ? 5.0f : 1.0f);
	}


	return SnapToSurface + TryMove(LeftOver, InitialDireciton, Pos + SnapToSurface, GravityPass, Depth + 1);
}

bool MoveComponent::GetIsOnGround() const
{
	return GroundedTimer > 0;
}

Vector3 MoveComponent::GetVelocity() const
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
	if (InputDirection.Length() > 1)
	{
		InputDirection = InputDirection.Normalize();
	}

	MovementVelocity += Vector2(InputDirection.X, InputDirection.Z) * Performance::DeltaTime * Acceleration;

	float InputLength = InputDirection.Length();
	float MovementLength = MovementVelocity.Length();

	if (MovementLength > MaxSpeed * InputLength)
	{
		if (MovementLength > MaxSpeed && InputLength >= 0.95f)
		{
			MovementVelocity = MovementVelocity.Normalize() * MaxSpeed;
		}
		else
		{
			MovementLength = std::max(MovementLength - Deceleration * Performance::DeltaTime, 0.0f);
			MovementVelocity = MovementVelocity.Normalize() * MovementLength;
		}
	}

	Vector3 MoveDir = Vector3(MovementVelocity.X, 0, MovementVelocity.Y).ProjectToPlane(0, GroundNormal) * Performance::DeltaTime;
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
