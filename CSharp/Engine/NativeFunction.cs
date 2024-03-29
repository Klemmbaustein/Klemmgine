﻿using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Engine.Native;

public static class NativeFunction
{
	static readonly Dictionary<string, IntPtr> LoadedNativeFunctions = [];

	public static void UnloadAll()
	{
		LoadedNativeFunctions.Clear();
	}

	public static void RegisterNativeFunction(string Name, IntPtr FunctionPtr)
	{
		LoadedNativeFunctions.Add(Name, FunctionPtr);
	}
	public static object CallNativeFunction(string Name, Type del, object[] Args)
	{
		if (!LoadedNativeFunctions.TryGetValue(Name, out nint Value))
		{
			Log.Print("Failed to call native function: " + Name, Log.Severity.Error);
			Log.Print("------------------------ Native functions: ------------------------", Log.Severity.Error);
			foreach (var i in LoadedNativeFunctions)
			{
				Log.Print(i.Key, Log.Severity.Error);
			}
			return null;
		}

		var NewDel = Marshal.GetDelegateForFunctionPointer(Value, del);

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
