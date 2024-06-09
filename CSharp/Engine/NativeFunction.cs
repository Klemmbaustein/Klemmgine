using System;
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

	private static void NativeFunctionError(string Message)
	{
		Log.Print(Message, Log.Severity.Error);
		Log.Print("------------------------ Native functions: ------------------------", Log.Severity.Error);
		foreach (var i in LoadedNativeFunctions)
		{
			Log.Print(i.Key, Log.Severity.Error);
		}
	}

	public static object CallNativeFunction(string Name, Type DelegateType, object[] Args)
	{
		if (!DelegateType.IsSubclassOf(typeof(Delegate)))
		{
			Log.Print($"Attempted to call a native function but the given type doesn't derive from Delegate. Type: {DelegateType}", Log.Severity.Error);
			return null;
		}

		if (!LoadedNativeFunctions.TryGetValue(Name, out IntPtr FunctionPointer))
		{
			NativeFunctionError($"Failed to call native function: {Name}");
			return null;
		}

		Delegate PointerDelegate = Marshal.GetDelegateForFunctionPointer(FunctionPointer, DelegateType);

		if (PointerDelegate == null)
		{
			NativeFunctionError($"Failed to get delegate for native function: {Name}");
			return null;
		}

		return PointerDelegate.DynamicInvoke(Args);
	}
}
