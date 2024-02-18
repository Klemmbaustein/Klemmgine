using System;
using System.Runtime.InteropServices;
using Engine.Native;
namespace Engine;

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
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), [ NativePtr, Parent.NativePtr ]);
		}
		NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewParticleComponent", typeof(NewParticle), [ FileName, Parent.NativePtr ]);
	}

	public override void Tick()
	{
	}
}