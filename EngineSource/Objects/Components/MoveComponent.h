#pragma once
#include <Objects/Components/Component.h>

class MoveComponent : public Component
{
	// Tries to move the object in the given direction.
	// If "TryAdjust" is "true", it will try to adjust the object's position to one that is more valid.
	Vector3 TryMove(Vector3 Direction, Vector3 InitialDireciton, Vector3 Pos, bool GravityPass, int Depth = 0);

	const int MoveMaxDepth = 5;

	Vector2 MovementVelocity;
	bool CanJump = true;
	float VerticalVelocity = 0;
	Vector3 InputDirection;
	Vector3 GroundNormal = Vector3(0, 1, 0);
	int GroundedTimer = 0;
	bool HasBounced = false;
	bool Jumping = false;
	void* CollisionBodyPtr = nullptr;
public:
	Vector2 ColliderSize = Vector2(1.0f, 1.0f);
	// Returns true if the object is touching the ground, false if not.
	bool GetIsOnGround() const;
	Vector3 GetVelocity() const;
	void Begin() override;
	void Update() override;
	void Destroy() override;
	void AddMovementInput(Vector3 Direction);

	// Sets the vertical velocity of the player to the "JumpHeight"
	void Jump();

	// Parameters for the movement
	float JumpHeight = 35;
	float MaxSpeed = 15;
	float Acceleration = 200;
	float Deceleration = 150;
	float AirAccelMultiplier = 0.5f;
	float Gravity = 100;
	bool Active = true;
};