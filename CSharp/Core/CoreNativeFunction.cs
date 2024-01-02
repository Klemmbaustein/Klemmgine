using System;
using System.Runtime.InteropServices;
#nullable enable

public static class CoreNativeFunction
{
	static Dictionary<string, IntPtr> LoadedNativeFunctions = new Dictionary<string, IntPtr>();

	public static void RegisterNativeFunction([MarshalAs(UnmanagedType.LPUTF8Str)] string Name, IntPtr FunctionPtr)
	{
		if (Engine.LoadedAsm == null)
		{
			return;
		}
		Engine.LoadTypeFromAssembly("NativeFunction")!.GetMethod("RegisterNativeFunction")!.Invoke(null, new object[] { Name, FunctionPtr });
		LoadedNativeFunctions.Add(Name, FunctionPtr);
	}

	public static object? CallNativeFunction(string Name, Type del, object?[]? Args)
	{
		var NewDel = Marshal.GetDelegateForFunctionPointer(LoadedNativeFunctions[Name], del);

		if (NewDel == null)
		{
			return null;
		}

		return NewDel.DynamicInvoke(Args);
	}
	public static void UnloadNativeFunctions()
	{
		LoadedNativeFunctions.Clear();
	}

}
