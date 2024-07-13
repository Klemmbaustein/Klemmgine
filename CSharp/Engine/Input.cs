using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using Engine.Native;

namespace Engine
{
	public static class Input
	{
		public enum Key
		{
			UNKNOWN = 0,

			RETURN = '\r',
			ESCAPE = '\x1B',
			BACKSPACE = '\b',
			TAB = '\t',
			SPACE = ' ',
			EXCLAIM = '!',
			QUOTEDBL = '"',
			HASH = '#',
			PERCENT = '%',
			DOLLAR = '$',
			AMPERSAND = '&',
			QUOTE = '\'',
			LEFTPAREN = '(',
			RIGHTPAREN = ')',
			ASTERISK = '*',
			PLUS = '+',
			COMMA = ',',
			MINUS = '-',
			PERIOD = '.',
			SLASH = '/',
			KEY_0 = '0',
			KEY_1 = '1',
			KEY_2 = '2',
			KEY_3 = '3',
			KEY_4 = '4',
			KEY_5 = '5',
			KEY_6 = '6',
			KEY_7 = '7',
			KEY_8 = '8',
			KEY_9 = '9',
			COLON = ':',
			SEMICOLON = ';',
			LESS = '<',
			EQUALS = '=',
			GREATER = '>',
			QUESTION = '?',
			AT = '@',

			/*
			   Skip uppercase letters
			 */

			LEFTBRACKET = '[',
			BACKSLASH = '\\',
			RIGHTBRACKET = ']',
			CARET = '^',
			UNDERSCORE = '_',
			BACKQUOTE = '`',
			a = 'a',
			b = 'b',
			c = 'c',
			d = 'd',
			e = 'e',
			f = 'f',
			g = 'g',
			h = 'h',
			i = 'i',
			j = 'j',
			k = 'k',
			l = 'l',
			m = 'm',
			n = 'n',
			o = 'o',
			p = 'p',
			q = 'q',
			r = 'r',
			s = 's',
			t = 't',
			u = 'u',
			v = 'v',
			w = 'w',
			x = 'x',
			y = 'y',
			z = 'z',

			F1 = 0x4000003A,
			F2 = 0x4000003B,
			F3 = 0x4000003C,
			F4 = 0x4000003D,
			F5 = 0x4000003E,
			F6 = 0x4000003F,
			F7 = 0x40000040,
			F8 = 0x40000041,
			F9 = 0x40000042,
			F10 = 0x40000043,
			F11 = 0x40000044,
			F12 = 0x40000045,

			LCTRL = 0x400000E0,
			LSHIFT = 0x400000E1,
			LALT = 0x400000E2,
			LGUI = 0x400000E3,
			RCTRL = 0x400000E4,
			RSHIFT = 0x400000E5,
			RALT = 0x400000E6,
			RGUI = 0x400000E7,

			RIGHT = 0x4000004F,
			LEFT = 0x40000050,
			DOWN = 0x40000051,
			UP = 0x40000052,
		}

		private delegate bool NativeIsKeyDown(int k);
		private delegate Vector3 NativeGetMouseMovement();
		private delegate void SetCursorVisibleDelegate(bool NewVisible);
		private delegate Int32 GetNumGamepads();
		private delegate Gamepad GetGamepadIndex(Int32 GamepadIndex);
		[return: MarshalAs(UnmanagedType.U1)]
		private delegate bool GetBool();

		public static bool IsKeyDown(Key k)
		{
			return (bool)NativeFunction.CallNativeFunction("IsKeyDown", typeof(NativeIsKeyDown), new object[] { k });
		}

		public static Vector3 GetMouseMovement()
		{
			return (Vector3)NativeFunction.CallNativeFunction("GetMouseMovement", typeof(NativeGetMouseMovement), null);
		}

		public static bool IsLMBDown
		{
			get
			{
				return (bool)NativeFunction.CallNativeFunction("GetIsLMBDown", typeof(GetBool), []);
			}
		}
		public static bool IsLMBClicked
		{
			get
			{
				return (bool)NativeFunction.CallNativeFunction("GetIsLMBClicked", typeof(GetBool), []);
			}
		}
		public static bool IsRMBDown
		{
			get
			{
				return (bool)NativeFunction.CallNativeFunction("GetIsRMBDown", typeof(GetBool), []);
			}
		}
		public static bool IsRMBClicked
		{
			get
			{
				return (bool)NativeFunction.CallNativeFunction("GetIsRMBClicked", typeof(GetBool), []);
			}
		}

		public static void SetCursorVisible(bool NewVisible)
		{
			NativeFunction.CallNativeFunction("SetCursorVisible", typeof(SetCursorVisibleDelegate), new object[] { NewVisible });
		}

		public static void UpdateGamepadList()
		{
			Int32 NumGamepads = (Int32)NativeFunction.CallNativeFunction("GetNumGamepads", typeof(GetNumGamepads), null);
			Gamepads.Clear();
			for (Int32 i = 0; i < NumGamepads; i++)
			{
				Gamepads.Add((Gamepad)NativeFunction.CallNativeFunction("GetGamepadIndex", typeof(GetGamepadIndex), new object[] { i }));
			}
		}

		public static List<Gamepad> Gamepads { get; private set; } = new();
		[StructLayout(LayoutKind.Sequential)]
		public struct Gamepad
		{
			public Gamepad()
			{

			}

			private readonly UInt32 ID = 0;
			public readonly Vector2 LeftStickPosition = 0, RightStickPosition = 0, DPadLocation = 0;

			[MarshalAs(UnmanagedType.LPStr)]
			public readonly string DeviceName;

			public readonly float LeftBumper = 0, RightBumper = 0;
			// Corresponds to bool* Input::Gamepad::Buttons in C++, this is only here so the struct has the same size
			private readonly IntPtr ButtonsArrayPtr;
		}
	}
}