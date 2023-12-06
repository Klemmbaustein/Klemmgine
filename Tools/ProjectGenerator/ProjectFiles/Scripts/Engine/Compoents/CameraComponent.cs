using System;
using System.Reflection;
using System.Runtime.InteropServices;

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
		NativeFunction.CallNativeFunction("UseCamera", typeof(UseCamera), new object[] { NativePtr });
	}

	public override void OnAttached()
	{
		if (Parent == null)
		{
			return;
		}

		if (!NativePtr.Equals(new IntPtr()))
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativeObject });
		}

		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewCameraComponent", typeof(NewCamera), new object[] { 60, Parent.NativeObject });
	}

	public override void Tick()
	{
	}

	public override void Destroy()
	{
		NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativeObject });
		NativePtr = new IntPtr();
	}
}