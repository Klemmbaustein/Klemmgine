#include "MoveComponent.h"
#include <Engine/Stats.h>
#include <Engine/Log.h>
#include <Networking/Client.h>


Collision::HitResponse MoveComponent::TryMove(Vector3 Direction, float Distance, bool TryAdjust)
{
	return Collision::HitResponse();
}

bool MoveComponent::GetIsOnGround()
{
    return IsOnGround;
}

Vector3 MoveComponent::GetVelocity()
{
	return Vector3(MovementVelocity.X, VerticalVelocity, MovementVelocity.Y);
}

void MoveComponent::Begin()
{
}

void MoveComponent::Update()
{
	if (IsInEditor || !Active || (GetParent()->GetIsReplicated() && (Client::GetIsConnected() && GetParent()->NetOwner != Client::GetClientID())))
	{
		return;
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

}
