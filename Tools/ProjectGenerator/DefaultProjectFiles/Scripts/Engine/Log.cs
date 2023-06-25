using System;
#nullable enable

public static class Log
{
	static System.Action<string>? PrintFunction = null;

	public static void LoadLogFunction(System.Action<string> Target)
	{
		PrintFunction = Target;
	}
	public static void Print(string Message)
	{
		if (PrintFunction == null)
		{
			return;
		}
		PrintFunction.Invoke(Message);
	}
}