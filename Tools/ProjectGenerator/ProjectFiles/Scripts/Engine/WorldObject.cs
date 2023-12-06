using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
#nullable enable

public abstract class WorldObject
{
	public IntPtr NativeObject = new();

	private delegate Int32 NewCSObjectDelegate(string TypeName, Transform t);
	private delegate Int32 DestroyObjectDelegate(IntPtr ObjPtr);
	private delegate void SetTransformDelegate(Transform NewTransform, IntPtr NativeObjectPtr);
	private delegate Transform GetTransformDelegate(IntPtr NativeObjectPtr);

	private static Delegate? GetCSObjectDelegate;

	public Vector3 GetPosition()
	{
		return GetTransform().Position;
	}

	public void SetPosition(Vector3 NewPosition)
	{
		Transform NewTransform = GetTransform();
		NewTransform.Position = NewPosition;
		SetTransform(NewTransform);
	}

	public Transform GetTransform()
	{
		return (Transform)NativeFunction.CallNativeFunction("GetObjectTransform", typeof(GetTransformDelegate), new object[] { NativeObject })!;
	}

	public void SetTransform(Transform NewTransform)
	{
		NativeFunction.CallNativeFunction("SetObjectTransform", typeof(SetTransformDelegate), new object[] { NewTransform, NativeObject });
	}

	List<ObjectComponent> AttachedComponents = new List<ObjectComponent>();

	public static void DestroyObject(WorldObject o)
	{
		NativeFunction.CallNativeFunction("DestroyObject", typeof(DestroyObjectDelegate), new object[] { o.NativeObject });
		return;
	}

	public static void LoadGetObjectFunction(Delegate Func)
	{
		GetCSObjectDelegate = Func;
	}

	public static WorldObject? NewCSObject(string TypeName, Transform t)
	{
		object? RetVal = NativeFunction.CallNativeFunction("NewCSObject", typeof(NewCSObjectDelegate), new object[] { TypeName, t });
		if (RetVal == null)
		{
			return null;
		}
		Int32 ObjID = (Int32)RetVal;
		if (GetCSObjectDelegate == null)
		{
			return null;
		}

		object? objRef = GetCSObjectDelegate.DynamicInvoke(ObjID);
		if (objRef == null)
		{
			return null;
		}
		return (WorldObject)objRef;
	}

	public void Attach(ObjectComponent c)
	{
		c.Parent = this;
		AttachedComponents.Add(c);
		c.OnAttached();
	}
	public void Detach(ObjectComponent c)
	{
		c.Destroy();
		AttachedComponents.Remove(c);
	}


	public void TickComponents()
	{
		foreach (var c in AttachedComponents)
		{
			c.Tick();
		}
	}

	public abstract void Tick();
	public abstract void Begin();
	protected abstract void Destroy();
	public void DestroyObjectInternal()
	{
		Destroy();
		foreach (var c in AttachedComponents)
		{
			c.Destroy();
		}
		AttachedComponents.Clear();
	}
}