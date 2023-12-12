using System;
#nullable enable

namespace Engine.Log
{
	public static class Log
	{
		public enum Severity
		{
			Info = 0,
			Warning = 1,
			Error = 2
		}

		static System.Action<string, int>? PrintFunction = null;

		public static void LoadLogFunction(System.Action<string, int> Target)
		{
			PrintFunction = Target;
		}
		public static void Print(string Message, Severity MessageSeverity = Severity.Info)
		{
			if (PrintFunction == null)
			{
				return;
			}
			PrintFunction.Invoke(Message, (int)MessageSeverity);
		}
	}
}