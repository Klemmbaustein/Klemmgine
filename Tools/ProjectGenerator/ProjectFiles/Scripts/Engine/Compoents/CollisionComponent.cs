using System;
using System.Runtime.InteropServices;

public class CollisionComponent : ObjectComponent
{
	private delegate IntPtr NewCollider([MarshalAs(UnmanagedType.LPUTF8Str)] string Filename, IntPtr Parent);


	public CollisionComponent()
	{
	}

	public void Load(string File)
	{
		if (Parent == null)
		{
			return;
		}

		if (!NativePtr.Equals(new IntPtr()))
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativeObject });
		}

		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewCollisionComponent", typeof(NewCollider), new object[] { File, Parent.NativeObject });
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