using System;

public class MoveComponent : ObjectComponent
{
	private delegate IntPtr NewMovement(IntPtr Parent);
	private delegate void AddMovementDelegate(Vector3 Direction, IntPtr NativeComponent);
	private delegate void JumpDelegate(IntPtr NativeComponent);

	public override void OnAttached()
	{
		base.OnAttached();
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewMoveComponent", typeof(NewMovement), new object[] { Parent.NativeObject });
	}

	public override void Tick()
	{

	}

	public void AddMovementInput(Vector3 Direction)
	{
		if (NativePtr != new IntPtr())
		{
			NativeFunction.CallNativeFunction("MovementComponentAddMovementInput", typeof(AddMovementDelegate), new object[] { Direction, NativePtr });
		}
	}

	public void Jump()
	{
		if (NativePtr != new IntPtr())
		{
			NativeFunction.CallNativeFunction("MovementComponentJump", typeof(JumpDelegate), new object[] { NativePtr });
		}
	}

	public override void Destroy()
	{
		if (NativePtr != new IntPtr())
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativeObject });
		}
		NativePtr = new IntPtr();
	}
}