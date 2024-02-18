using System;
using Engine.Native;
namespace Engine;

/**
 * @defgroup CSharp-Components
 * @ingroup CSharp-Objects
 * 
 * @brief
 * Functions/classes related to components in C#
 */

/**
 * @brief
 * WorldObject Component in C#.
 * 
 * Can be attached to any Engine.WorldObject.
 * 
 * C++ equivalent: Component.
 * 
 * @ingroup CSharp-Components
 */
public abstract class ObjectComponent
{
	// Points to the native version of this class
	public IntPtr NativePtr = new();

	public WorldObject Parent = null;
	protected delegate void DestroyComponent(IntPtr Component, IntPtr Parent);
	private delegate void SetTransformDelegate(IntPtr Component, Transform NewTransform);
	private delegate Transform GetTransformDelegate(IntPtr Component);


	public virtual void OnAttached() { }

	public Transform GetRelativeTransform()
	{
		if (NativePtr == 0)
		{
			return new Transform();
		}

		return (Transform)NativeFunction.CallNativeFunction("GetComponentTransform", typeof(GetTransformDelegate), [ NativePtr ]);
	}

	/**
	 * @brief
	 * Gets the native RelativeTransform.Position value of the component.
	 */
	public Vector3 GetRelativePosition()
	{
		return GetRelativeTransform().Position;
	}

	/**
	 * @brief
	 * Sets the native RelativeTransform.Position value of the component.
	 */
	public void SetRelativePosition(Vector3 NewPosition)
	{
		Transform NewTransform = GetRelativeTransform();
		NewTransform.Position = NewPosition;
		SetRelativeTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native RelativeTransform.Rotation value of the component.
	 */
	public Vector3 GetRelativeRotation()
	{
		return GetRelativeTransform().Rotation;
	}

	/**
	 * @brief
	 * Sets the native RelativeTransform.Rotation value of the component.
	 */
	public void SetRelativeRotation(Vector3 NewRotation)
	{
		Transform NewTransform = GetRelativeTransform();
		NewTransform.Position = NewRotation;
		SetRelativeTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native Transform.Scale value of the component.
	 */
	public Vector3 GetRelativeScale()
	{
		return GetRelativeTransform().Scale;
	}

	/**
	 * @brief
	 * Sets the native Transform.Scale value of the component.
	 */
	public void SetRelativeScale(Vector3 NewScale)
	{
		Transform NewTransform = GetRelativeTransform();
		NewTransform.Position = NewScale;
		SetRelativeTransform(NewTransform);
	}

	public void SetRelativeTransform(Transform t)
	{
		if (NativePtr == 0)
		{
			return;
		}

		NativeFunction.CallNativeFunction("SetComponentTransform", typeof(SetTransformDelegate), [ NativePtr, t ]);
	}

	public abstract void Tick();

	public virtual void Destroy()
	{
		if (NativePtr != 0)
		{
			NativeFunction.CallNativeFunction("DestroyComponent", typeof(DestroyComponent), [ NativePtr, Parent.NativePtr ]);
		}
		NativePtr = 0;
	}
}