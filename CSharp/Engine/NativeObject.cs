
using System;
using System.Runtime.InteropServices;

namespace Engine;

public class NativeObject : WorldObject
{
	delegate NativeType GetPropertyTypeDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	delegate string GetPropertyStringDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	delegate int GetPropertyIntDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	[return: MarshalAs(UnmanagedType.U1)]
	delegate bool GetPropertyBoolDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	delegate Vector3 GetPropertyVector3Delegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	public void LoadFromPtr(IntPtr Pointer)
	{
		NativePtr = Pointer;
	}

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
