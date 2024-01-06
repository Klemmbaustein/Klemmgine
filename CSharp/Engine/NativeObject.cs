
using System;
using System.Runtime.InteropServices;

namespace Engine;

public class NativeObject : WorldObject
{
	delegate NativeType GetPropertyTypeDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	delegate string GetPropertyStringDelegate(IntPtr obj, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

	public void LoadFromPtr(IntPtr Pointer)
	{
		NativeObject = Pointer;
	}

	public object GetProperty(string Name)
	{
		object[] args = [2];
		args[0] = NativeObject;
		args[1] = Name;
		NativeType Type = (NativeType)NativeFunction.CallNativeFunction("GetObjectPropertyType",
			typeof(GetPropertyTypeDelegate), 
			args);

		switch (Type)
		{
			case NativeType.String:
				return NativeFunction.CallNativeFunction("GetObjectPropertyString",
					typeof(GetPropertyStringDelegate),
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
