using Engine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
#nullable enable

namespace Engine;

/**
 * @defgroup CSharp-Objects
 * @ingroup CSharp
 * 
 * @brief
 * Functions/classes related to objects in C#
 */

/**
 * @brief
 * C# class representing an object.
 * 
 * C++ equivalent: WorldObject class.
 * 
 * @ingroup CSharp-Objects
 * @todo Implement networking functions in C#.
 */
public abstract class WorldObject
{
	public IntPtr NativeObject = new();

	private delegate Int32 NewCSObjectDelegate(string TypeName, Transform t);
	private delegate Int32 DestroyObjectDelegate(IntPtr ObjPtr);
	private delegate void SetTransformDelegate(Transform NewTransform, IntPtr NativeObjectPtr);
	private delegate Transform GetTransformDelegate(IntPtr NativeObjectPtr);

	private static Delegate? GetCSObjectDelegate;


	/**
	 * @brief
	 * Spawns a new CSharpObject with the given type and transform.
	 * 
	 * @param TypeName
	 * The name of the type that the new object should have.
	 * 
	 * @param t
	 * The Transform of the new object.
	 */
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

	/**
	 * @brief
	 * Gets the native Transform.Position value of the object.
	 */
	public Vector3 GetPosition()
	{
		return GetTransform().Position;
	}

	/**
	 * @brief
	 * Sets the native Transform.Position value of the object.
	 */
	public void SetPosition(Vector3 NewPosition)
	{
		Transform NewTransform = GetTransform();
		NewTransform.Position = NewPosition;
		SetTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native Transform.Rotation value of the object.
	 */
	public Vector3 GetRotation()
	{
		return GetTransform().Rotation;
	}

	/**
	 * @brief
	 * Sets the native Transform.Rotation value of the object.
	 */
	public void SetRotation(Vector3 NewRotation)
	{
		Transform NewTransform = GetTransform();
		NewTransform.Position = NewRotation;
		SetTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native Transform.Scale value of the object.
	 */
	public Vector3 GetScale()
	{
		return GetTransform().Scale;
	}

	/**
	 * @brief
	 * Sets the native Transform.Scale value of the object.
	 */
	public void SetScale(Vector3 NewScale)
	{
		Transform NewTransform = GetTransform();
		NewTransform.Position = NewScale;
		SetTransform(NewTransform);
	}

	/**
	 * @brief
	 * Gets the native Transform value of the object.
	 */
	public Transform GetTransform()
	{
		return (Transform)NativeFunction.CallNativeFunction("GetObjectTransform", typeof(GetTransformDelegate), new object[] { NativeObject })!;
	}
	/**
	 * @brief
	 * Sets the native Transform value of the object.
	 */
	public void SetTransform(Transform NewTransform)
	{
		NativeFunction.CallNativeFunction("SetObjectTransform", typeof(SetTransformDelegate), new object[] { NewTransform, NativeObject });
	}

	readonly List<ObjectComponent> AttachedComponents = [];

	/**
	 * @brief
	 * Destroys the given object.
	 * 
	 * @param o
	 * The object to destroy.
	 */
	public static void DestroyObject(WorldObject o)
	{
		NativeFunction.CallNativeFunction("DestroyObject", typeof(DestroyObjectDelegate), new object[] { o.NativeObject });
		return;
	}

	public static void LoadGetObjectFunction(Delegate Func)
	{
		GetCSObjectDelegate = Func;
	}

	/**
	 * @brief
	 * Attaches the given component to the object.
	 */
	public void Attach(ObjectComponent c)
	{
		c.Parent = this;
		AttachedComponents.Add(c);
		c.OnAttached();
	}

	/**
	 * @brief
	 * Deletes and detaches the given component from the object.
	 */
	public void Detach(ObjectComponent c)
	{
		c.Destroy();
		AttachedComponents.Remove(c);
	}


	public void UpdateComponents()
	{
		foreach (var c in AttachedComponents)
		{
			c.Tick();
		}
	}

	/**
	 * @brief
	 * Function called each frame.
	 * 
	 * Each object has to implement this function.
	 */
	public abstract void Update();
	
	/**
	 * @brief
	 * Function called once the object is created.
	 * 
	 * Each object has to implement this function.
	 */
	public abstract void Begin();
	
	/**
	 * @brief
	 * Function called if the object is destroyed.
	 * 
	 * Each object has to implement this function.
	 */
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