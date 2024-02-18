using System;
using Engine.Native;
namespace Engine;

public class MoveComponent : ObjectComponent
{
	private delegate IntPtr NewMovement(IntPtr Parent);
	private delegate void AddMovementDelegate(Vector3 Direction, IntPtr NativeComponent);
	private delegate void JumpDelegate(IntPtr NativeComponent);

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

	public void Jump()
	{
		if (NativePtr != new IntPtr())
		{
			NativeFunction.CallNativeFunction("MovementComponentJump", typeof(JumpDelegate), [ NativePtr ]);
		}
	}
}