using Engine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
#nullable enable

public static class NativeFunction
{
	static Dictionary<string, IntPtr> LoadedNativeFunctions = new Dictionary<string, IntPtr>();

	public static void RegisterNativeFunction(string Name, IntPtr FunctionPtr)
	{
		LoadedNativeFunctions.Add(Name, FunctionPtr);
	}
	public static object? CallNativeFunction(string Name, Type del, object?[]? Args)
	{
		if (!LoadedNativeFunctions.ContainsKey(Name))
		{
			Log.Print("Failed to call native function: " + Name, Log.Severity.Error);
			Log.Print("------------------------ Native functions: ------------------------", Log.Severity.Error);
			foreach (var i in LoadedNativeFunctions)
			{
				Log.Print(i.Key, Log.Severity.Error);
			}
			return null;
		}

		var NewDel = Marshal.GetDelegateForFunctionPointer(LoadedNativeFunctions[Name], del);

		if (NewDel == null)
		{
			Log.Print("Failed to get delegate for native function: " + Name, Log.Severity.Error);
			Log.Print("------------------------ Native functions: ------------------------", Log.Severity.Error);
			foreach (var i in LoadedNativeFunctions)
			{
				Log.Print(i.Key, Log.Severity.Error);
			}
			return null;
		}

		return NewDel.DynamicInvoke(Args);
	}
}
