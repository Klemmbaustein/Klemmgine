#pragma once
#include <Objects/Components/Component.h>

/**
* @brief
* A component for movement.
* 
* Contains code for basic movement behavior.
* 
* @ingroup Components
*/
class MoveComponent : public Component
{
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
	bool CollideStatic = true;

	// Tries to move the object in the given direction.
	// If "TryAdjust" is "true", it will try to adjust the object's position to one that is more valid.
	Vector3 TryMove(Vector3 Direction, Vector3 InitialDirection, Vector3 Pos, bool GravityPass, int Depth = 0);

	/// The size of the collider used by the movement. X is the capsule width, Y is the capsule height.
	Vector2 ColliderSize = Vector2(1.0f, 1.0f);
	/// Returns true if the object is touching the ground, false if not.
	bool GetIsOnGround() const;

	/// Gets the velocity the movement is at.
	Vector3 GetVelocity() const;
	void Begin() override;
	void Update() override;
	void Destroy() override;
	/**
	* @brief
	* Adds an input to the movement. The movement will try to move in this direction.
	*/
	void AddMovementInput(Vector3 Direction);

	/// Sets the vertical velocity of the player to the "JumpHeight"
	void Jump();

	/// Jump height.
	float JumpHeight = 35;
	/// The maximum movement speed.
	float MaxSpeed = 15;
	/// The acceleration of the movement.
	float Acceleration = 200;
	/// The deceleration of the movement.
	float Deceleration = 150;
	/// The air acceleration multiplier. If the movement is in air, both acceleration and deceleration will be multipled with this value.
	float AirAccelMultiplier = 0.5f;
	/// The gravity applied to the movement.
	float Gravity = 100;
	/// True if the movement is active.
	bool Active = true;
	bool LastMoveSuccessful = false;
};