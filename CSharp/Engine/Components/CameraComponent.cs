using System;
using Engine.Native;
namespace Engine;

public class CameraComponent : ObjectComponent
{
	private delegate IntPtr NewCamera(float FOV, IntPtr Parent);
	private delegate void UseCamera(IntPtr Cam);
	private delegate float GetCameraFOV(IntPtr Cam);
	private delegate void SetCameraFOV(IntPtr Cam, float NewFOV);

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

	public float FOV
	{ 
		get
		{
			return (float)NativeFunction.CallNativeFunction("GetCameraFOV", typeof(GetCameraFOV), [ NativePtr ]);
		}
		set
		{
			NativeFunction.CallNativeFunction("SetCameraFOV", typeof(SetCameraFOV), [NativePtr, value]);
		}
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

		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewCameraComponent", typeof(NewCamera), [ 70, Parent.NativePtr ]);
	}

	public override void Tick()
	{
	}
}