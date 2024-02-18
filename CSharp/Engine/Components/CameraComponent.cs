using System;
using Engine.Native;
namespace Engine;

public class CameraComponent : ObjectComponent
{
	private delegate IntPtr NewCamera(float FOV, IntPtr Parent);
	private delegate void UseCamera(IntPtr Cam);


	public CameraComponent()
	{
	}
	

	public void Use()
	{
		if (NativePtr.Equals(new IntPtr()))
		{
			return;
		}
		NativeFunction.CallNativeFunction("UseCamera", typeof(UseCamera), [ NativePtr ]);
	}

	public override void OnAttached()
	{
		if (Parent == null)
		{
			return;
		}

		if (!NativePtr.Equals(new IntPtr()))
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), [ NativePtr, Parent.NativePtr ]);
		}

		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewCameraComponent", typeof(NewCamera), [ 60, Parent.NativePtr ]);
	}

	public override void Tick()
	{
	}
}