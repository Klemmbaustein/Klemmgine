using System;
#nullable enable

namespace Engine
{
	/**
	 * @brief
	 * Logging class. Prints messages to standard output and Log.
	 * 
	 * Equivalent to the Log:: namespace in <Engine/Log.h>.
	 * 
	 * Note: Log::PrintMultiLine() is not supported in C#.
	 * 
	 * @ingroup CSharp
	 */
	public class Log
	{
		/**
		 * @brief
		 * Severity type. Controls color of a log message.
		 * 
		 * Controls color of a log message printed by Log.Print().
		 */
		public enum Severity
		{
			/// Prints as white.
			Info = 0,
			/// Prints as yellow.
			Warning = 1,
			/// Prints as red.
			Error = 2
		}

		static System.Action<string, int>? PrintFunction = null;

		public static void LoadLogFunction(System.Action<string, int> Target)
		{
			PrintFunction = Target;
		}

		/**
		 * @brief
		 * Mostly equivalent to Log::Print() in <Engine/Log.h>. Prints the string Message to standard output and log if avaliable.
		 * 
		 * There are a few differences between this function and Log::Print() in <Engine/Log.h>.
		 * 1. The string `[C#]: [Script]: ` gets printed before `Message`.
		 * 2. You cannot specify the color of the message. Instead, you can define the severity with the Log.Severity enum. Color
		 *    is determined by the severity.
		 * 
		 * This function calls the native function `CSharpInternalPrint()` in `CSharp/CSharpInterop.cpp`, which calls `CSharp::CSharpLog()`.
		 * `CSharp::CSharpLog()` calls Log::Print().
		 */
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