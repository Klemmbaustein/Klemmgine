#include "MoveComponent.h"
#include <World/Stats.h>
#include <Engine/Log.h>

/*
* This movement code is somewhat fucky, but hey, It works.
*/

Collision::HitResponse MoveComponent::TryMove(Vector3 Direction, float Distance, bool TryAdjust)
{
	Vector3 PreviousLocation = GetParent()->GetTransform().Location;
	Direction = Direction.Normalize();
	Distance = Distance * Performance::DeltaTime;
	auto DownTrace = Collision::LineTrace(GetParent()->GetTransform().Location, GetParent()->GetTransform().Location + Direction * Distance);
	uint8_t Index = TryAdjust;
	for (float Time = 0; Distance > 0 ? (Time < Distance) : (Time > Distance); Time = std::min(Distance, Time + (Distance > 0 ? 0.5f : -0.5f)))
	{
		CollisionMeshes[Index]->RelativeTransform.Location = Direction * Time;
		auto MoveHit = CollisionMeshes[Index]->OverlapCheck(std::set({ CollisionMeshes[0], CollisionMeshes[1] }));
		if (MoveHit.Hit)
		{
			MoveHit.Normal.Y += 0.1;
			MoveHit.Normal.Normalize();
			if (TryAdjust)
			{
				Vector3 Dir = MoveHit.Normal * abs(Distance) * 0.3 + Vector3(Vector3::Dot(MoveHit.Normal, Vector3(0, 1, 0)) * abs(Distance) * 0.1);
				if (TryMove(Dir, Dir.Length(), false).Hit)
				{
					GetParent()->GetTransform().Location += Dir;
					MoveHit.Normal = Vector3(0, 1, 0);
				}
				else
				{
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
		Vector3(1,  0,  0),
		Vector3(-1,  0,  0),
		Vector3(0,  1,  0),
		Vector3(0, -1,  0),
		Vector3(0,  0,  1),
		Vector3(0,  0, -1)
	};
	CollisionModel.Elements.push_back(ModelGenerator::ModelData::Element());
	for (auto dir : CubeDirections)
	{
		CollisionModel.Elements[0].AddFace(2, dir, 0);
	}
	CollisionModel.MakeCollisionBox();
	CollisionMeshes[0] = new CollisionComponent();
	GetParent()->Attach(CollisionMeshes[0]);
	CollisionMeshes[0]->Init(CollisionModel.GetMergedVertices(), CollisionModel.GetMergedIndices());
	CollisionMeshes[0]->RelativeTransform.Scale = Vector3(0.5, 1, 0.5);

	CollisionMeshes[1] = new CollisionComponent();
	GetParent()->Attach(CollisionMeshes[1]);
	CollisionMeshes[1]->Init(CollisionModel.GetMergedVertices(), CollisionModel.GetMergedIndices());
	CollisionMeshes[1]->RelativeTransform.Scale = Vector3(0.5, 0.8, 0.5);
}

void MoveComponent::Tick()
{
	if (IsInEditor || !Active)
	{
		return;
	}
	InputDirection.Y = 0;
	InputDirection.Normalize();
	MovementVelocity.X += InputDirection.X * Performance::DeltaTime * Velocity * (IsOnGround ? 1 : 0.1);
	MovementVelocity.Y += InputDirection.Z * Performance::DeltaTime * Velocity * (IsOnGround ? 1 : 0.1);
	MovementVelocity *= IsOnGround ? powf(0.5, Performance::DeltaTime * 50) : powf(0.5, Performance::DeltaTime * 5);
	TryMove(Vector3(MovementVelocity.X, 0, MovementVelocity.Y), MovementVelocity.Length());
	auto MoveHit = TryMove(Vector3(0, 1, 0), VerticalVelocity, false);
	if (MoveHit.Hit && (Vector3::Dot(MoveHit.Normal, Vector3(0, 1, 0)) > 0.4))
	{
		VerticalVelocity = std::max(-5.0f, VerticalVelocity);
		CanJump = true;
		IsOnGround = 15;
		HasBounced = false;
	}
	else if (MoveHit.Hit)
	{
		GetParent()->GetTransform().Location += MoveHit.Normal * 0.2 + Vector3(0, Performance::DeltaTime, 0);
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
	InputDirection = 0;
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
		TryMove(Vector3(0, 1, 0), 0.1 / Performance::DeltaTime);
		CanJump = false;
	}
}
