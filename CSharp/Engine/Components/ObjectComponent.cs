using System;
using Engine.Native;
namespace Engine;

/**
 * @defgroup CSharp-Components C# Components
 * @ingroup CSharp-Objects
 * 
 * @brief
 * Functions/classes related to components in C#
 */

/**
 * @brief
 * SceneObject Component in C#.
 * 
 * Can be attached to any Engine.SceneObject.
 * 
 * C++ equivalent: Component.
 * 
 * @ingroup CSharp-Components
 */
public abstract class ObjectComponent
{
	// Points to the native version of this class
	public IntPtr NativePtr = new();

	public SceneObject Parent = null;
	protected delegate void DestroyComponent(IntPtr Component, IntPtr Parent);
	private delegate void SetTransformDelegate(IntPtr Component, Transform NewTransform);
	private delegate Transform GetTransformDelegate(IntPtr Component);


	public virtual void OnAttached() { }

	/**
	 * @brief
	 * The native RelativeTransform value of the component.
	 */
	public Transform RelativeTransform
	{
		get
		{
			if (NativePtr == 0)
			{
				return new Transform();
			}

			return (Transform)NativeFunction.CallNativeFunction("GetComponentTransform", typeof(GetTransformDelegate), [NativePtr]);
		}
		set
		{
			if (NativePtr == 0)
			{
				return;
			}

			NativeFunction.CallNativeFunction("SetComponentTransform", typeof(SetTransformDelegate), [NativePtr, value]);

		}
	}

	/**
	 * @brief
	 * The native RelativeTransform.Position value of the component.
	 */
	public Vector3 RelativePosition
	{
		get
		{
			return RelativeTransform.Position;
		}
		set
		{
			Transform NewTransform = RelativeTransform;
			NewTransform.Position = value;
			RelativeTransform = NewTransform;

		}
	}

	/**
	 * @brief
	 * The native RelativeTransform.Rotation value of the component.
	 */
	public Vector3 RelativeRotation
	{
		get
		{
			return RelativeTransform.Rotation;
		}
		set
		{
			Transform NewTransform = RelativeTransform;
			NewTransform.Rotation = value;
			RelativeTransform = NewTransform;
		}
	}

	/**
	 * @brief
	 * The native RelativeTransform.Scale value of the component.
	 */
	public Vector3 RelativeScale
	{
		get
		{
			return RelativeTransform.Scale;
		}
		set
		{
			Transform NewTransform = RelativeTransform;
			NewTransform.Scale = value;
			RelativeTransform = NewTransform;
		}
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