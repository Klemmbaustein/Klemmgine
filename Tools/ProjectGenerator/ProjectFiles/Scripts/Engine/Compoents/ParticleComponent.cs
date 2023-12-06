using System;
using System.Runtime.InteropServices;

public class ParticleComponent : ObjectComponent
{
	private delegate IntPtr NewParticle([MarshalAs(UnmanagedType.LPUTF8Str)] string Filename, IntPtr Parent);


	public void Load(string FileName)
	{
		if (Parent == null)
		{
			return;
		}
		if (!NativePtr.Equals(new IntPtr()))
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativeObject });
		}
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewParticleComponent", typeof(NewParticle), new object[] { FileName, Parent.NativeObject });
	}

	public override void Destroy()
	{
		NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), new object[] { NativePtr, Parent.NativeObject });
		NativePtr = new IntPtr();
	}

	public override void Tick()
	{
	}
}