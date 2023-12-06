using System;

public abstract class ObjectComponent
{
	// Points to the native version of this class
	protected IntPtr NativePtr = new();

	public WorldObject Parent = null;
	protected delegate void DestroyComponent(IntPtr Component, IntPtr Parent);
	private delegate void SetTransformDelegate(IntPtr Component, Transform NewTransform);
	private delegate Transform GetTransformDelegate(IntPtr Component);


	public virtual void OnAttached() { }

	public Transform GetTransform()
	{
		return (Transform)NativeFunction.CallNativeFunction("GetComponentTransform", typeof(GetTransformDelegate), new object[] { NativePtr });
	}

	public void SetRelativePosition(Vector3 NewPos)
	{
		if (NativePtr.Equals(new IntPtr()))
		{
			return;
		}
		Transform t = GetTransform();
		t.Position = NewPos;
		SetRelativeTransform(t);
	}

	public void SetRelativeTransform(Transform t)
	{
		NativeFunction.CallNativeFunction("SetComponentTransform", typeof(SetTransformDelegate), new object[] { NativePtr, t });
	}

	public abstract void Tick();

	public abstract void Destroy();
}