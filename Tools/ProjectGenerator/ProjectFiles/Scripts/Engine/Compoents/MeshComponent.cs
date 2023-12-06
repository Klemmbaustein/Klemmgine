using System;
using System.Reflection;
using System.Runtime.InteropServices;

public class MeshComponent : ObjectComponent
{
	private delegate IntPtr NewModel([MarshalAs(UnmanagedType.LPUTF8Str)] string Filename, IntPtr Parent);


	public MeshComponent()
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
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewMeshComponent", typeof(NewModel), new object[] { File, Parent.NativeObject });
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