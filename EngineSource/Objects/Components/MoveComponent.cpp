#include "MoveComponent.h"
#include <Engine/Stats.h>
#include <Engine/Log.h>
#include <Networking/Client.h>

// TODO: Rewrite
// TODO: Rewrite again because this is really bad

Collision::HitResponse MoveComponent::TryMove(Vector3 Direction, float Distance, bool TryAdjust)
{
	Vector3 PreviousLocation = GetParent()->GetTransform().Location;
	Direction = Direction.Normalize();
	Distance = Distance * Performance::DeltaTime;
	uint8_t Index = TryAdjust;
	for (float Time = 0; Distance > 0 ? (Time < Distance) : (Time > Distance); Time = std::min(Distance, Time + (Distance > 0 ? 0.5f : -0.5f)))
	{
		CollisionMeshes[Index]->RelativeTransform.Location = Direction * Time;
		auto MoveHit = CollisionMeshes[Index]->OverlapCheck(std::set({ CollisionMeshes[0], CollisionMeshes[1] }));
		if (MoveHit.Hit)
		{
			MoveHit.Normal = (MoveHit.Normal + Vector3(0, 0.1f, 0)).Normalize();
			if (TryAdjust)
			{
				Vector3 Dir = MoveHit.Normal * abs(Distance) * 0.3f + Vector3(Vector3::Dot(MoveHit.Normal, Vector3(0, 1, 0)) * abs(Distance) * 0.1f);
				if (TryMove(Dir, Dir.Length(), false).Hit)
				{
					GetParent()->GetTransform().Location += Dir;
					MoveHit.Normal = Vector3(0, 1, 0);
				}
			}
			return MoveHit;
		}
		else
		{
			GetParent()->GetTransform().Location = PreviousLocation + Direction * Time;
		}
	}
	CollisionMeshes[Index]->RelativeTransform.Location = 0;
	GetParent()->GetTransform().Location = PreviousLocation + Direction * Distance;
	return Collision::HitResponse();
}

bool MoveComponent::GetIsOnGround()
{
    return IsOnGround;
}

Vector3 MoveComponent::GetVelocity()
{
	return Vector3(MovementVelocity.X, VerticalVelocity + 5, MovementVelocity.Y);
}

void MoveComponent::Begin()
{
	ModelGenerator::ModelData CollisionModel;
	Vector3 CubeDirections[6] =
	{
		Vector3( 1,  0,  0),
		Vector3(-1,  0,  0),
		Vector3( 0,  1,  0),
		Vector3( 0, -1,  0),
		Vector3( 0,  0,  1),
		Vector3( 0,  0, -1)
	};
	CollisionModel.Elements.push_back(ModelGenerator::ModelData::Element());
	for (auto dir : CubeDirections)
	{
		CollisionModel.Elements[0].AddFace(2, dir, 0);
	}
	CollisionModel.MakeCollisionBox();
	CollisionMeshes[0] = new CollisionComponent();
	GetParent()->Attach(CollisionMeshes[0]);
	CollisionMeshes[0]->Load(CollisionModel);
	CollisionMeshes[0]->RelativeTransform.Scale = Vector3(0.5f, 1.0f, 0.5f);

	CollisionMeshes[1] = new CollisionComponent();
	GetParent()->Attach(CollisionMeshes[1]);
	CollisionMeshes[1]->Load(CollisionModel);
	CollisionMeshes[1]->RelativeTransform.Scale = Vector3(0.5f, 0.8f, 0.5f);
}

void MoveComponent::Update()
{
	if (IsInEditor || !Active || (GetParent()->GetIsReplicated() && (Client::GetIsConnected() && GetParent()->NetOwner != Client::GetClientID())))
	{
		return;
	}
	InputDirection.Y = 0;
	float Length = InputDirection.Length();
	InputDirection = InputDirection * Vector3(powf(Length, 2.0f));
	if (Length > 1)
	{
		InputDirection = InputDirection.Normalize();
		Length = 1;
	}

	MovementVelocity.X += InputDirection.X * Performance::DeltaTime * Velocity * (IsOnGround ? 1 : 0.1f);
	MovementVelocity.Y += InputDirection.Z * Performance::DeltaTime * Velocity * (IsOnGround ? 1 : 0.1f);
	MovementVelocity *= IsOnGround ? powf(0.5f, Performance::DeltaTime * 50) : powf(0.5f, Performance::DeltaTime * 5);
	InputDirection = 0;
	TryMove(Vector3(MovementVelocity.X, 0, MovementVelocity.Y), MovementVelocity.Length());
	auto MoveHit = TryMove(Vector3(0, 1, 0), VerticalVelocity, false);
	if (MoveHit.Hit && (Vector3::Dot(MoveHit.Normal, Vector3(0, 1, 0)) > 0.4f))
	{
		VerticalVelocity = std::max(-5.0f, VerticalVelocity);
		CanJump = true;
		IsOnGround = 15;
		HasBounced = false;
	}
	else if (MoveHit.Hit)
	{
		GetParent()->GetTransform().Location += MoveHit.Normal * 0.2f + Vector3(0, Performance::DeltaTime, 0);
		if (!HasBounced)
		{
			auto ReflectedDirection = glm::reflect((glm::vec3)Vector3(MovementVelocity.X, VerticalVelocity, MovementVelocity.Y), (glm::vec3)MoveHit.Normal);
			MovementVelocity = Vector2(ReflectedDirection.x, ReflectedDirection.y) / 2;
			VerticalVelocity = ReflectedDirection.y;
			HasBounced = true;
		}
		VerticalVelocity += Performance::DeltaTime * -Gravity;
		if (IsOnGround > 0) IsOnGround--;
	}
	else
	{
		if (IsOnGround > 0) IsOnGround--;
		VerticalVelocity += Performance::DeltaTime * -Gravity;
		if (MoveHit.Hit)
		{
			TryMove(Vector3(0, 1, 0), -1, true);
			// Move the player with collision avoidance
			// if we are touching a wall that we can't stand on
		}
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
	if (CanJump)
	{
		VerticalVelocity = JumpHeight;
		TryMove(Vector3(0, 1, 0), 0.1f / Performance::DeltaTime);
		CanJump = false;
	}
}
