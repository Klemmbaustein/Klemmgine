using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;


public static class EngineLog
{
	[return: MarshalAs(UnmanagedType.LPUTF8Str)]
	public delegate void LogDelegate(string message);
	static LogDelegate? PrintFunction = null;


	public static void LoadLogFunction(IntPtr LoadedFunction)
	{
		PrintFunction = Marshal.GetDelegateForFunctionPointer<LogDelegate>(LoadedFunction);
	}

	public static void Print(string Message)
	{
		// Allow the program to crash here of 'PrintFunction' is null
		// If PrintFunction is null, this means that something is wrong anyways.
		PrintFunction!(Message);
	}
}