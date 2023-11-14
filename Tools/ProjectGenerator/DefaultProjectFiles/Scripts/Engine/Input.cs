
namespace Engine.Input
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

		public static bool IsKeyDown(Key k)
		{
			return (bool)NativeFunction.CallNativeFunction("IsKeyDown", typeof(NativeIsKeyDown), new object[] { k });
		}

		public static Vector3 GetMouseMovement()
		{
			return (Vector3)NativeFunction.CallNativeFunction("GetMouseMovement", typeof(NativeGetMouseMovement), null);
		}

		public static void SetCursorVisible(bool NewVisible)
		{
			NativeFunction.CallNativeFunction("SetCursorVisible", typeof(SetCursorVisibleDelegate), new object[] { NewVisible });
		}
	}
}