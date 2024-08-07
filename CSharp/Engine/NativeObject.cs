﻿
using System;
using System.Runtime.InteropServices;

namespace Engine;
using Engine.Native;

/**
 * @brief
 * Native object. Represents a native C++ object.
 * 
 * @ingroup CSharp-Objects
 */
public class NativeObject : SceneObject
{
	delegate NativeType GetPropertyTypeDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	delegate string GetPropertyStringDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	delegate int GetPropertyIntDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	[return: MarshalAs(UnmanagedType.U1)]
	delegate bool GetPropertyBoolDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	delegate Vector3 GetPropertyVector3Delegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);
	delegate IntPtr NewNativeObjectDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string Name, Transform t);

	/**
	 * @brief
	 * Sets the pointer to the native object this object represents.
	 */
	public void LoadFromPtr(IntPtr Pointer)
	{
		NativePtr = Pointer;
	}

	/**
	 * @brief
	 * Gets the value of a C++ property from the given name.
	 */
	public object GetProperty(string Name)
	{
		object[] args = [NativePtr, Name];
		NativeType Type = (NativeType)NativeFunction.CallNativeFunction("GetObjectPropertyType",
			typeof(GetPropertyTypeDelegate), 
			args);
		switch (Type)
		{
			case NativeType.String:
				return NativeFunction.CallNativeFunction("GetObjectPropertyString",
					typeof(GetPropertyStringDelegate),
					args);
			case NativeType.Int:
				return NativeFunction.CallNativeFunction("GetObjectPropertyInt",
					typeof(GetPropertyIntDelegate),
					args);
			case NativeType.Bool:
				return NativeFunction.CallNativeFunction("GetObjectPropertyBool",
					typeof(GetPropertyBoolDelegate),
					args);
			case NativeType.Vector3:
				return NativeFunction.CallNativeFunction("GetObjectPropertyVector3",
					typeof(GetPropertyVector3Delegate),
					args);
			case NativeType.Float:
				return NativeFunction.CallNativeFunction("GetObjectPropertyFloat",
					typeof(GetPropertyVector3Delegate),
					args);
			default:
				break;
		}
		return null;
	}

	public static NativeObject New(string TypeName, Transform t)
	{
		IntPtr NativePtr = (IntPtr)NativeFunction.CallNativeFunction("NewNativeObject",
			typeof(NewNativeObjectDelegate),
			[TypeName, t]);

		var Object = new NativeObject();
		Object.LoadFromPtr(NativePtr);
		return Object;
	}

	public override void Begin()
	{
	}

	public override void Update()
	{
	}

	protected override void Destroy()
	{
	}
}
