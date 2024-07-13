using System;
using Engine.Native;
namespace Engine;

/**
* @brief
* A component for movement.
* 
* Contains code for basic movement behavior.
* 
* @ingroup CSharp-Components
*/
public class MoveComponent : ObjectComponent
{
	private delegate IntPtr NewMovement(IntPtr Parent);
	private delegate void AddMovementDelegate(Vector3 Direction, IntPtr NativeComponent);
	private delegate void JumpDelegate(IntPtr NativeComponent);
	private delegate void SetFloatDelegate(IntPtr NativeComponent, float Value);
	private delegate void GetBoolDelegate(IntPtr NativeComponent, float Value);

	public override void OnAttached()
	{
		base.OnAttached();
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewMoveComponent", typeof(NewMovement), [Parent.NativePtr]);
	}

	public override void Tick()
	{

	}
	/**
	* @brief
	* Adds an input to the movement. The movement will try to move in this direction.
	*/
	public void AddMovementInput(Vector3 Direction)
	{
		if (NativePtr == 0)
		{
			return;
		}
		NativeFunction.CallNativeFunction("MovementComponentAddMovementInput", typeof(AddMovementDelegate), [Direction, NativePtr]);
	}

	/// The maximum movement speed.
	public float MoveSpeed
	{
		set
		{
			if (NativePtr == 0)
			{
				return;
			}
			NativeFunction.CallNativeFunction("MoveComponentSetMoveSpeed", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}
	/// The acceleration of the movement.
	public float Acceleration
	{
		set
		{
			if (NativePtr == 0)
			{
				return;
			}

			NativeFunction.CallNativeFunction("MoveComponentSetAcceleration", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}
	/// The deceleration of the movement.
	public float Deceleration
	{
		set
		{
			if (NativePtr == 0)
			{
				return;
			}

			NativeFunction.CallNativeFunction("MoveComponentSetDeceleration", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	/// The air acceleration multiplier. If the movement is in air, both acceleration and deceleration will be multipled with this value.
	public float AirAccel
	{
		set
		{
			if (NativePtr == 0)
			{
				return;
			}

			NativeFunction.CallNativeFunction("MoveComponentSetAirAccel", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	/// The gravity applied to the movement.
	public float Gravity
	{
		set
		{
			if (NativePtr == 0)
			{
				return;
			}

			NativeFunction.CallNativeFunction("MoveComponentSetGravity", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	/// True if the object is touching the ground, false if not.
	public bool OnGround
	{
		get
		{
			if (NativePtr == 0)
			{
				return false;
			}

			return (bool)NativeFunction.CallNativeFunction("MoveComponentIsOnGround", typeof(GetBoolDelegate), [NativePtr]);
		}
	}

	/// Jump height.
	public float JumpHeight
	{
		set
		{
			if (NativePtr == 0)
			{
				return;
			}

			NativeFunction.CallNativeFunction("MoveComponentSetJumpHeight", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	/// Sets the vertical velocity of the player to the "JumpHeight"
	public void Jump()
	{
		if (NativePtr == 0)
		{
			return;
		}
		NativeFunction.CallNativeFunction("MovementComponentJump", typeof(JumpDelegate), [NativePtr]);
	}
}