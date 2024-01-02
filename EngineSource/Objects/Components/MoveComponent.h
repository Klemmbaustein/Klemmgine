#pragma once
#include <Objects/Components/Component.h>
#include <Objects/Components/CollisionComponent.h>

class MoveComponent : public Component
{
	// Tries to move the object in the given direction, over the given distance.
	// If "TryAdjust" is "true", it will try to adjust the object's position to one that is more valid.
	Collision::HitResponse TryMove(Vector3 Direction, float Distance, bool TryAdjust = true);

	Vector2 MovementVelocity;
	bool CanJump = true;
	float VerticalVelocity = 0;
	Vector3 InputDirection;
	int IsOnGround = 5;
	bool HasBounced = false;
public:
	CollisionComponent* CollisionMeshes[2] = { nullptr, nullptr };
	// Returns true if the object is touching the ground, false if not.
	bool GetIsOnGround();
	Vector3 GetVelocity();
	void Begin() override;
	void Update() override;
	void Destroy() override;
	void AddMovementInput(Vector3 Direction);

	// Sets the vertical velocity of the player to the "JumpHeight"
	void Jump();

	// Parameters for the movement
	float Velocity = 1000;
	float JumpHeight = 35;
	float Gravity = 125;
	bool Active = true;
};