using System;
using Engine.Native;
namespace Engine;

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
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewMoveComponent", typeof(NewMovement), [ Parent.NativePtr ]);
	}

	public override void Tick()
	{

	}

	public void AddMovementInput(Vector3 Direction)
	{
		if (NativePtr != new IntPtr())
		{
			NativeFunction.CallNativeFunction("MovementComponentAddMovementInput", typeof(AddMovementDelegate), [ Direction, NativePtr ]);
		}
	}

	public float MoveSpeed
	{
		set
		{
			NativeFunction.CallNativeFunction("MoveComponentSetMoveSpeed", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}
	public float Acceleration
	{
		set
		{
			NativeFunction.CallNativeFunction("MoveComponentSetAcceleration", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}
	public float Deceleration
	{
		set
		{
			NativeFunction.CallNativeFunction("MoveComponentSetDeceleration", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	public float AirAccel
	{
		set
		{
			NativeFunction.CallNativeFunction("MoveComponentSetAirAccel", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	public float Gravity
	{
		set
		{
			NativeFunction.CallNativeFunction("MoveComponentSetGravity", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	public bool OnGround
	{
		get
		{
			return (bool)NativeFunction.CallNativeFunction("MoveComponentIsOnGround", typeof(GetBoolDelegate), [NativePtr]);
		}
	}

	public float JumpHeight
	{
		set
		{
			NativeFunction.CallNativeFunction("MoveComponentSetJumpHeight", typeof(SetFloatDelegate), [NativePtr, value]);
		}
	}

	public void Jump()
	{
		if (NativePtr != new IntPtr())
		{
			NativeFunction.CallNativeFunction("MovementComponentJump", typeof(JumpDelegate), [ NativePtr ]);
		}
	}
}