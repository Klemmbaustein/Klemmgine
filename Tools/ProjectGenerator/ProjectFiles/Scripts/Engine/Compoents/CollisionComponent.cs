using Engine;
using Engine.Log;
using System;
using System.Runtime.InteropServices;

public class CollisionComponent : ObjectComponent
{
	private delegate IntPtr NewCollider([MarshalAs(UnmanagedType.LPUTF8Str)] string Filename, IntPtr Parent);
	private delegate Collision.HitResponse CollisionCheckDelegate(
		[MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] IntPtr[] ComponentsToIgnore,
		int Length,
		IntPtr NativeType);

	public CollisionComponent()
	{
	}

	public Collision.HitResponse CollisionCheck(CollisionComponent[] ComponentsToIgnore = null)
	{
		if (!NativePtr.Equals(new IntPtr()))
		{
			if (ComponentsToIgnore == null)
			{
				return (Collision.HitResponse)NativeFunction.CallNativeFunction("CollisionComponentOverlap",
					typeof(CollisionCheckDelegate),
					new object[] { new IntPtr[] { new() }, 0, NativePtr });

			}
			IntPtr[] ComponentPtrs = new IntPtr[ComponentsToIgnore.Length];

			for (int i = 0; i < ComponentsToIgnore.Length; i++)
			{
				ComponentPtrs[i] = ComponentsToIgnore[i].NativePtr;
			}

			return (Collision.HitResponse)NativeFunction.CallNativeFunction("CollisionComponentOverlap",
				typeof(CollisionCheckDelegate),
				new object[] { ComponentPtrs, ComponentPtrs.Length, NativePtr });
		}
		return new Collision.HitResponse();
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