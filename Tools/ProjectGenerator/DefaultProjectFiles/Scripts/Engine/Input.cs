using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public static class Input
{
	public enum Key
	{
		SDLK_UNKNOWN = 0,

		SDLK_RETURN = '\r',
		SDLK_ESCAPE = '\x1B',
		SDLK_BACKSPACE = '\b',
		SDLK_TAB = '\t',
		SDLK_SPACE = ' ',
		SDLK_EXCLAIM = '!',
		SDLK_QUOTEDBL = '"',
		SDLK_HASH = '#',
		SDLK_PERCENT = '%',
		SDLK_DOLLAR = '$',
		SDLK_AMPERSAND = '&',
		SDLK_QUOTE = '\'',
		SDLK_LEFTPAREN = '(',
		SDLK_RIGHTPAREN = ')',
		SDLK_ASTERISK = '*',
		SDLK_PLUS = '+',
		SDLK_COMMA = ',',
		SDLK_MINUS = '-',
		SDLK_PERIOD = '.',
		SDLK_SLASH = '/',
		SDLK_0 = '0',
		SDLK_1 = '1',
		SDLK_2 = '2',
		SDLK_3 = '3',
		SDLK_4 = '4',
		SDLK_5 = '5',
		SDLK_6 = '6',
		SDLK_7 = '7',
		SDLK_8 = '8',
		SDLK_9 = '9',
		SDLK_COLON = ':',
		SDLK_SEMICOLON = ';',
		SDLK_LESS = '<',
		SDLK_EQUALS = '=',
		SDLK_GREATER = '>',
		SDLK_QUESTION = '?',
		SDLK_AT = '@',

		/*
		   Skip uppercase letters
		 */

		SDLK_LEFTBRACKET = '[',
		SDLK_BACKSLASH = '\\',
		SDLK_RIGHTBRACKET = ']',
		SDLK_CARET = '^',
		SDLK_UNDERSCORE = '_',
		SDLK_BACKQUOTE = '`',
		SDLK_a = 'a',
		SDLK_b = 'b',
		SDLK_c = 'c',
		SDLK_d = 'd',
		SDLK_e = 'e',
		SDLK_f = 'f',
		SDLK_g = 'g',
		SDLK_h = 'h',
		SDLK_i = 'i',
		SDLK_j = 'j',
		SDLK_k = 'k',
		SDLK_l = 'l',
		SDLK_m = 'm',
		SDLK_n = 'n',
		SDLK_o = 'o',
		SDLK_p = 'p',
		SDLK_q = 'q',
		SDLK_r = 'r',
		SDLK_s = 's',
		SDLK_t = 't',
		SDLK_u = 'u',
		SDLK_v = 'v',
		SDLK_w = 'w',
		SDLK_x = 'x',
		SDLK_y = 'y',
		SDLK_z = 'z',

		SDLK_F1 = 0x4000003A,
		SDLK_F2 = 0x4000003B,
		SDLK_F3 = 0x4000003C,
		SDLK_F4 = 0x4000003D,
		SDLK_F5 = 0x4000003E,
		SDLK_F6 = 0x4000003F,
		SDLK_F7 = 0x40000040,
		SDLK_F8 = 0x40000041,
		SDLK_F9 = 0x40000042,
		SDLK_F10 = 0x40000043,
		SDLK_F11 = 0x40000044,
		SDLK_F12 = 0x40000045,

		SDLK_LCTRL = 0x400000E0,
		SDLK_LSHIFT = 0x400000E1,
		SDLK_LALT = 0x400000E2,
		SDLK_LGUI = 0x400000E3,
		SDLK_RCTRL = 0x400000E4,
		SDLK_RSHIFT = 0x400000E5,
		SDLK_RALT = 0x400000E6,
		SDLK_RGUI = 0x400000E7,

		SDLK_RIGHT = 0x4000004F,
		SDLK_LEFT = 0x40000050,
		SDLK_DOWN = 0x40000051,
		SDLK_UP = 0x40000052,
	}

	private delegate bool NativeIsKeyDown(int k);
	private delegate Vector3 NativeGetMouseMovement();

	public static bool IsKeyDown(Key k)
	{
		return (bool)NativeFunction.CallNativeFunction("IsKeyDown", typeof(NativeIsKeyDown), new object[] { k });
	}

	public static Vector3 GetMouseMovement()
	{
		return (Vector3)NativeFunction.CallNativeFunction("GetMouseMovement", typeof(NativeGetMouseMovement), null);
	}
}