#pragma once
#include <Math/Vector.h>

#define SDLK_SCANCODE_MASK (1<<30)
#define TO_KEYCODE(X)  ((int)X | SDLK_SCANCODE_MASK)

namespace Input
{
	extern Vector2 MouseMovement;
	extern bool IsLMBDown;
	extern bool IsRMBDown;
	extern bool CursorVisible;
	extern Vector2 MouseLocation;
	extern bool BlockInput;

    /* SDL2 Scancodes/KeyCodes but de-SDLified. This way no projecet needs to include SDL.h by itself. */

	enum class Key_Scancode
    {
        SCANCODE_UNKNOWN = 0,

        /**
         *  \name Usage page 0x07
         *
         *  These values are from usage page 0x07 (USB keyboard page).
         */
         /* @{ */

        SCANCODE_A = 4,
        SCANCODE_B = 5,
        SCANCODE_C = 6,
        SCANCODE_D = 7,
        SCANCODE_E = 8,
        SCANCODE_F = 9,
        SCANCODE_G = 10,
        SCANCODE_H = 11,
        SCANCODE_I = 12,
        SCANCODE_J = 13,
        SCANCODE_K = 14,
        SCANCODE_L = 15,
        SCANCODE_M = 16,
        SCANCODE_N = 17,
        SCANCODE_O = 18,
        SCANCODE_P = 19,
        SCANCODE_Q = 20,
        SCANCODE_R = 21,
        SCANCODE_S = 22,
        SCANCODE_T = 23,
        SCANCODE_U = 24,
        SCANCODE_V = 25,
        SCANCODE_W = 26,
        SCANCODE_X = 27,
        SCANCODE_Y = 28,
        SCANCODE_Z = 29,

        SCANCODE_1 = 30,
        SCANCODE_2 = 31,
        SCANCODE_3 = 32,
        SCANCODE_4 = 33,
        SCANCODE_5 = 34,
        SCANCODE_6 = 35,
        SCANCODE_7 = 36,
        SCANCODE_8 = 37,
        SCANCODE_9 = 38,
        SCANCODE_0 = 39,

        SCANCODE_RETURN = 40,
        SCANCODE_ESCAPE = 41,
        SCANCODE_BACKSPACE = 42,
        SCANCODE_TAB = 43,
        SCANCODE_SPACE = 44,

        SCANCODE_MINUS = 45,
        SCANCODE_EQUALS = 46,
        SCANCODE_LEFTBRACKET = 47,
        SCANCODE_RIGHTBRACKET = 48,
        SCANCODE_BACKSLASH = 49, /**< Located at the lower left of the return
                                      *   key on ISO keyboards and at the right end
                                      *   of the QWERTY row on ANSI keyboards.
                                      *   Produces REVERSE SOLIDUS (backslash) and
                                      *   VERTICAL LINE in a US layout, REVERSE
                                      *   SOLIDUS and VERTICAL LINE in a UK Mac
                                      *   layout, NUMBER SIGN and TILDE in a UK
                                      *   Windows layout, DOLLAR SIGN and POUND SIGN
                                      *   in a Swiss German layout, NUMBER SIGN and
                                      *   APOSTROPHE in a German layout, GRAVE
                                      *   ACCENT and POUND SIGN in a French Mac
                                      *   layout, and ASTERISK and MICRO SIGN in a
                                      *   French Windows layout.
                                      */
        SCANCODE_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                      *   instead of 49 for the same key, but all
                                      *   OSes I've seen treat the two codes
                                      *   identically. So, as an implementor, unless
                                      *   your keyboard generates both of those
                                      *   codes and your OS treats them differently,
                                      *   you should generate SCANCODE_BACKSLASH
                                      *   instead of this code. As a user, you
                                      *   should not rely on this code because SDL
                                      *   will never generate it with most (all?)
                                      *   keyboards.
                                      */
        SCANCODE_SEMICOLON = 51,
        SCANCODE_APOSTROPHE = 52,
        SCANCODE_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                                  *   and ISO keyboards). Produces GRAVE ACCENT and
                                  *   TILDE in a US Windows layout and in US and UK
                                  *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                                  *   and NOT SIGN in a UK Windows layout, SECTION
                                  *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                                  *   layouts on ISO keyboards, SECTION SIGN and
                                  *   DEGREE SIGN in a Swiss German layout (Mac:
                                  *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                                  *   DEGREE SIGN in a German layout (Mac: only on
                                  *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                                  *   French Windows layout, COMMERCIAL AT and
                                  *   NUMBER SIGN in a French Mac layout on ISO
                                  *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                                  *   SIGN in a Swiss German, German, or French Mac
                                  *   layout on ANSI keyboards.
                                  */
        SCANCODE_COMMA = 54,
        SCANCODE_PERIOD = 55,
        SCANCODE_SLASH = 56,

        SCANCODE_CAPSLOCK = 57,

        SCANCODE_F1 = 58,
        SCANCODE_F2 = 59,
        SCANCODE_F3 = 60,
        SCANCODE_F4 = 61,
        SCANCODE_F5 = 62,
        SCANCODE_F6 = 63,
        SCANCODE_F7 = 64,
        SCANCODE_F8 = 65,
        SCANCODE_F9 = 66,
        SCANCODE_F10 = 67,
        SCANCODE_F11 = 68,
        SCANCODE_F12 = 69,

        SCANCODE_PRINTSCREEN = 70,
        SCANCODE_SCROLLLOCK = 71,
        SCANCODE_PAUSE = 72,
        SCANCODE_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                       does send code 73, not 117) */
        SCANCODE_HOME = 74,
        SCANCODE_PAGEUP = 75,
        SCANCODE_DELETE = 76,
        SCANCODE_END = 77,
        SCANCODE_PAGEDOWN = 78,
        SCANCODE_RIGHT = 79,
        SCANCODE_LEFT = 80,
        SCANCODE_DOWN = 81,
        SCANCODE_UP = 82,

        SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                         */
        SCANCODE_KP_DIVIDE = 84,
        SCANCODE_KP_MULTIPLY = 85,
        SCANCODE_KP_MINUS = 86,
        SCANCODE_KP_PLUS = 87,
        SCANCODE_KP_ENTER = 88,
        SCANCODE_KP_1 = 89,
        SCANCODE_KP_2 = 90,
        SCANCODE_KP_3 = 91,
        SCANCODE_KP_4 = 92,
        SCANCODE_KP_5 = 93,
        SCANCODE_KP_6 = 94,
        SCANCODE_KP_7 = 95,
        SCANCODE_KP_8 = 96,
        SCANCODE_KP_9 = 97,
        SCANCODE_KP_0 = 98,
        SCANCODE_KP_PERIOD = 99,

        SCANCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                            *   keyboards have over ANSI ones,
                                            *   located between left shift and Y.
                                            *   Produces GRAVE ACCENT and TILDE in a
                                            *   US or UK Mac layout, REVERSE SOLIDUS
                                            *   (backslash) and VERTICAL LINE in a
                                            *   US or UK Windows layout, and
                                            *   LESS-THAN SIGN and GREATER-THAN SIGN
                                            *   in a Swiss German, German, or French
                                            *   layout. */
        SCANCODE_APPLICATION = 101, /**< windows contextual menu, compose */
        SCANCODE_POWER = 102, /**< The USB document says this is a status flag,
                                   *   not a physical key - but some Mac keyboards
                                   *   do have a power key. */
        SCANCODE_KP_EQUALS = 103,
        SCANCODE_F13 = 104,
        SCANCODE_F14 = 105,
        SCANCODE_F15 = 106,
        SCANCODE_F16 = 107,
        SCANCODE_F17 = 108,
        SCANCODE_F18 = 109,
        SCANCODE_F19 = 110,
        SCANCODE_F20 = 111,
        SCANCODE_F21 = 112,
        SCANCODE_F22 = 113,
        SCANCODE_F23 = 114,
        SCANCODE_F24 = 115,
        SCANCODE_EXECUTE = 116,
        SCANCODE_HELP = 117,    /**< AL Integrated Help Center */
        SCANCODE_MENU = 118,    /**< Menu (show menu) */
        SCANCODE_SELECT = 119,
        SCANCODE_STOP = 120,    /**< AC Stop */
        SCANCODE_AGAIN = 121,   /**< AC Redo/Repeat */
        SCANCODE_UNDO = 122,    /**< AC Undo */
        SCANCODE_CUT = 123,     /**< AC Cut */
        SCANCODE_COPY = 124,    /**< AC Copy */
        SCANCODE_PASTE = 125,   /**< AC Paste */
        SCANCODE_FIND = 126,    /**< AC Find */
        SCANCODE_MUTE = 127,
        SCANCODE_VOLUMEUP = 128,
        SCANCODE_VOLUMEDOWN = 129,
        /* not sure whether there's a reason to enable these */
        /*     SCANCODE_LOCKINGCAPSLOCK = 130,  */
        /*     SCANCODE_LOCKINGNUMLOCK = 131, */
        /*     SCANCODE_LOCKINGSCROLLLOCK = 132, */
        SCANCODE_KP_COMMA = 133,
        SCANCODE_KP_EQUALSAS400 = 134,

        SCANCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                                footnotes in USB doc */
        SCANCODE_INTERNATIONAL2 = 136,
        SCANCODE_INTERNATIONAL3 = 137, /**< Yen */
        SCANCODE_INTERNATIONAL4 = 138,
        SCANCODE_INTERNATIONAL5 = 139,
        SCANCODE_INTERNATIONAL6 = 140,
        SCANCODE_INTERNATIONAL7 = 141,
        SCANCODE_INTERNATIONAL8 = 142,
        SCANCODE_INTERNATIONAL9 = 143,
        SCANCODE_LANG1 = 144, /**< Hangul/English toggle */
        SCANCODE_LANG2 = 145, /**< Hanja conversion */
        SCANCODE_LANG3 = 146, /**< Katakana */
        SCANCODE_LANG4 = 147, /**< Hiragana */
        SCANCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
        SCANCODE_LANG6 = 149, /**< reserved */
        SCANCODE_LANG7 = 150, /**< reserved */
        SCANCODE_LANG8 = 151, /**< reserved */
        SCANCODE_LANG9 = 152, /**< reserved */

        SCANCODE_ALTERASE = 153,    /**< Erase-Eaze */
        SCANCODE_SYSREQ = 154,
        SCANCODE_CANCEL = 155,      /**< AC Cancel */
        SCANCODE_CLEAR = 156,
        SCANCODE_PRIOR = 157,
        SCANCODE_RETURN2 = 158,
        SCANCODE_SEPARATOR = 159,
        SCANCODE_OUT = 160,
        SCANCODE_OPER = 161,
        SCANCODE_CLEARAGAIN = 162,
        SCANCODE_CRSEL = 163,
        SCANCODE_EXSEL = 164,

        SCANCODE_KP_00 = 176,
        SCANCODE_KP_000 = 177,
        SCANCODE_THOUSANDSSEPARATOR = 178,
        SCANCODE_DECIMALSEPARATOR = 179,
        SCANCODE_CURRENCYUNIT = 180,
        SCANCODE_CURRENCYSUBUNIT = 181,
        SCANCODE_KP_LEFTPAREN = 182,
        SCANCODE_KP_RIGHTPAREN = 183,
        SCANCODE_KP_LEFTBRACE = 184,
        SCANCODE_KP_RIGHTBRACE = 185,
        SCANCODE_KP_TAB = 186,
        SCANCODE_KP_BACKSPACE = 187,
        SCANCODE_KP_A = 188,
        SCANCODE_KP_B = 189,
        SCANCODE_KP_C = 190,
        SCANCODE_KP_D = 191,
        SCANCODE_KP_E = 192,
        SCANCODE_KP_F = 193,
        SCANCODE_KP_XOR = 194,
        SCANCODE_KP_POWER = 195,
        SCANCODE_KP_PERCENT = 196,
        SCANCODE_KP_LESS = 197,
        SCANCODE_KP_GREATER = 198,
        SCANCODE_KP_AMPERSAND = 199,
        SCANCODE_KP_DBLAMPERSAND = 200,
        SCANCODE_KP_VERTICALBAR = 201,
        SCANCODE_KP_DBLVERTICALBAR = 202,
        SCANCODE_KP_COLON = 203,
        SCANCODE_KP_HASH = 204,
        SCANCODE_KP_SPACE = 205,
        SCANCODE_KP_AT = 206,
        SCANCODE_KP_EXCLAM = 207,
        SCANCODE_KP_MEMSTORE = 208,
        SCANCODE_KP_MEMRECALL = 209,
        SCANCODE_KP_MEMCLEAR = 210,
        SCANCODE_KP_MEMADD = 211,
        SCANCODE_KP_MEMSUBTRACT = 212,
        SCANCODE_KP_MEMMULTIPLY = 213,
        SCANCODE_KP_MEMDIVIDE = 214,
        SCANCODE_KP_PLUSMINUS = 215,
        SCANCODE_KP_CLEAR = 216,
        SCANCODE_KP_CLEARENTRY = 217,
        SCANCODE_KP_BINARY = 218,
        SCANCODE_KP_OCTAL = 219,
        SCANCODE_KP_DECIMAL = 220,
        SCANCODE_KP_HEXADECIMAL = 221,

        SCANCODE_LCTRL = 224,
        SCANCODE_LSHIFT = 225,
        SCANCODE_LALT = 226, /**< alt, option */
        SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
        SCANCODE_RCTRL = 228,
        SCANCODE_RSHIFT = 229,
        SCANCODE_RALT = 230, /**< alt gr, option */
        SCANCODE_RGUI = 231, /**< windows, command (apple), meta */

        SCANCODE_MODE = 257,    /**< I'm not sure if this is really not covered
                                     *   by any of the above, but since there's a
                                     *   special KMOD_MODE for it I'm adding it here
                                     */

                                     /* @} *//* Usage page 0x07 */

                                     /**
                                      *  \name Usage page 0x0C
                                      *
                                      *  These values are mapped from usage page 0x0C (USB consumer page).
                                      *  See https://usb.org/sites/default/files/hut1_2.pdf
                                      *
                                      *  There are way more keys in the spec than we can represent in the
                                      *  current scancode range, so pick the ones that commonly come up in
                                      *  real world usage.
                                      */
                                      /* @{ */

        SCANCODE_AUDIONEXT = 258,
        SCANCODE_AUDIOPREV = 259,
        SCANCODE_AUDIOSTOP = 260,
        SCANCODE_AUDIOPLAY = 261,
        SCANCODE_AUDIOMUTE = 262,
        SCANCODE_MEDIASELECT = 263,
        SCANCODE_WWW = 264,             /**< AL Internet Browser */
        SCANCODE_MAIL = 265,
        SCANCODE_CALCULATOR = 266,      /**< AL Calculator */
        SCANCODE_COMPUTER = 267,
        SCANCODE_AC_SEARCH = 268,       /**< AC Search */
        SCANCODE_AC_HOME = 269,         /**< AC Home */
        SCANCODE_AC_BACK = 270,         /**< AC Back */
        SCANCODE_AC_FORWARD = 271,      /**< AC Forward */
        SCANCODE_AC_STOP = 272,         /**< AC Stop */
        SCANCODE_AC_REFRESH = 273,      /**< AC Refresh */
        SCANCODE_AC_BOOKMARKS = 274,    /**< AC Bookmarks */

        /* @} *//* Usage page 0x0C */

        /**
         *  \name Walther keys
         *
         *  These are values that Christian Walther added (for mac keyboard?).
         */
         /* @{ */

        SCANCODE_BRIGHTNESSDOWN = 275,
        SCANCODE_BRIGHTNESSUP = 276,
        SCANCODE_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                               switch, video mode switch */
        SCANCODE_KBDILLUMTOGGLE = 278,
        SCANCODE_KBDILLUMDOWN = 279,
        SCANCODE_KBDILLUMUP = 280,
        SCANCODE_EJECT = 281,
        SCANCODE_SLEEP = 282,           /**< SC System Sleep */

        SCANCODE_APP1 = 283,
        SCANCODE_APP2 = 284,

        /* @} *//* Walther keys */

        /**
         *  \name Usage page 0x0C (additional media keys)
         *
         *  These values are mapped from usage page 0x0C (USB consumer page).
         */
         /* @{ */

        SCANCODE_AUDIOREWIND = 285,
        SCANCODE_AUDIOFASTFORWARD = 286,

        /* @} *//* Usage page 0x0C (additional media keys) */

        /**
         *  \name Mobile keys
         *
         *  These are values that are often used on mobile phones.
         */
         /* @{ */

        SCANCODE_SOFTLEFT = 287, /**< Usually situated below the display on phones and
                                          used as a multi-function feature key for selecting
                                          a software defined function shown on the bottom left
                                          of the display. */
        SCANCODE_SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                           used as a multi-function feature key for selecting
                                           a software defined function shown on the bottom right
                                           of the display. */
        SCANCODE_CALL = 289, /**< Used for accepting phone calls. */
        SCANCODE_ENDCALL = 290, /**< Used for rejecting phone calls. */

        /* @} *//* Mobile keys */

        /* Add any other keys here. */

        SDL_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                     for array bounds */
    };

	enum class Key
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
		k0 = '0',
		k1 = '1',
		k2 = '2',
		k3 = '3',
		k4 = '4',
		k5 = '5',
		k6 = '6',
		k7 = '7',
		k8 = '8',
		k9 = '9',
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

		CAPSLOCK = TO_KEYCODE(Key_Scancode::SCANCODE_CAPSLOCK),

		F1 = TO_KEYCODE(Key_Scancode::SCANCODE_F1),
		F2 = TO_KEYCODE(Key_Scancode::SCANCODE_F2),
		F3 = TO_KEYCODE(Key_Scancode::SCANCODE_F3),
		F4 = TO_KEYCODE(Key_Scancode::SCANCODE_F4),
		F5 = TO_KEYCODE(Key_Scancode::SCANCODE_F5),
		F6 = TO_KEYCODE(Key_Scancode::SCANCODE_F6),
		F7 = TO_KEYCODE(Key_Scancode::SCANCODE_F7),
		F8 = TO_KEYCODE(Key_Scancode::SCANCODE_F8),
		F9 = TO_KEYCODE(Key_Scancode::SCANCODE_F9),
		F10 = TO_KEYCODE(Key_Scancode::SCANCODE_F10),
		F11 = TO_KEYCODE(Key_Scancode::SCANCODE_F11),
		F12 = TO_KEYCODE(Key_Scancode::SCANCODE_F12),

		PRINTSCREEN = TO_KEYCODE(Key_Scancode::SCANCODE_PRINTSCREEN),
		SCROLLLOCK = TO_KEYCODE(Key_Scancode::SCANCODE_SCROLLLOCK),
		PAUSE = TO_KEYCODE(Key_Scancode::SCANCODE_PAUSE),
		INSERT = TO_KEYCODE(Key_Scancode::SCANCODE_INSERT),
		HOME = TO_KEYCODE(Key_Scancode::SCANCODE_HOME),
		PAGEUP = TO_KEYCODE(Key_Scancode::SCANCODE_PAGEUP),
		DELETE = '\x7F',
		END = TO_KEYCODE(Key_Scancode::SCANCODE_END),
		PAGEDOWN = TO_KEYCODE(Key_Scancode::SCANCODE_PAGEDOWN),
		RIGHT = TO_KEYCODE(Key_Scancode::SCANCODE_RIGHT),
		LEFT = TO_KEYCODE(Key_Scancode::SCANCODE_LEFT),
		DOWN = TO_KEYCODE(Key_Scancode::SCANCODE_DOWN),
		UP = TO_KEYCODE(Key_Scancode::SCANCODE_UP),

		NUMLOCKCLEAR = TO_KEYCODE(Key_Scancode::SCANCODE_NUMLOCKCLEAR),
		KP_DIVIDE = TO_KEYCODE(Key_Scancode::SCANCODE_KP_DIVIDE),
		KP_MULTIPLY = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MULTIPLY),
		KP_MINUS = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MINUS),
		KP_PLUS = TO_KEYCODE(Key_Scancode::SCANCODE_KP_PLUS),
		KP_ENTER = TO_KEYCODE(Key_Scancode::SCANCODE_KP_ENTER),
		KP_1 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_1),
		KP_2 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_2),
		KP_3 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_3),
		KP_4 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_4),
		KP_5 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_5),
		KP_6 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_6),
		KP_7 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_7),
		KP_8 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_8),
		KP_9 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_9),
		KP_0 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_0),
		KP_PERIOD = TO_KEYCODE(Key_Scancode::SCANCODE_KP_PERIOD),

		APPLICATION = TO_KEYCODE(Key_Scancode::SCANCODE_APPLICATION),
		POWER = TO_KEYCODE(Key_Scancode::SCANCODE_POWER),
		KP_EQUALS = TO_KEYCODE(Key_Scancode::SCANCODE_KP_EQUALS),
		F13 = TO_KEYCODE(Key_Scancode::SCANCODE_F13),
		F14 = TO_KEYCODE(Key_Scancode::SCANCODE_F14),
		F15 = TO_KEYCODE(Key_Scancode::SCANCODE_F15),
		F16 = TO_KEYCODE(Key_Scancode::SCANCODE_F16),
		F17 = TO_KEYCODE(Key_Scancode::SCANCODE_F17),
		F18 = TO_KEYCODE(Key_Scancode::SCANCODE_F18),
		F19 = TO_KEYCODE(Key_Scancode::SCANCODE_F19),
		F20 = TO_KEYCODE(Key_Scancode::SCANCODE_F20),
		F21 = TO_KEYCODE(Key_Scancode::SCANCODE_F21),
		F22 = TO_KEYCODE(Key_Scancode::SCANCODE_F22),
		F23 = TO_KEYCODE(Key_Scancode::SCANCODE_F23),
		F24 = TO_KEYCODE(Key_Scancode::SCANCODE_F24),
		EXECUTE = TO_KEYCODE(Key_Scancode::SCANCODE_EXECUTE),
		HELP = TO_KEYCODE(Key_Scancode::SCANCODE_HELP),
		MENU = TO_KEYCODE(Key_Scancode::SCANCODE_MENU),
		SELECT = TO_KEYCODE(Key_Scancode::SCANCODE_SELECT),
		STOP = TO_KEYCODE(Key_Scancode::SCANCODE_STOP),
		AGAIN = TO_KEYCODE(Key_Scancode::SCANCODE_AGAIN),
		UNDO = TO_KEYCODE(Key_Scancode::SCANCODE_UNDO),
		CUT = TO_KEYCODE(Key_Scancode::SCANCODE_CUT),
		COPY = TO_KEYCODE(Key_Scancode::SCANCODE_COPY),
		PASTE = TO_KEYCODE(Key_Scancode::SCANCODE_PASTE),
		FIND = TO_KEYCODE(Key_Scancode::SCANCODE_FIND),
		MUTE = TO_KEYCODE(Key_Scancode::SCANCODE_MUTE),
		VOLUMEUP = TO_KEYCODE(Key_Scancode::SCANCODE_VOLUMEUP),
		VOLUMEDOWN = TO_KEYCODE(Key_Scancode::SCANCODE_VOLUMEDOWN),
		KP_COMMA = TO_KEYCODE(Key_Scancode::SCANCODE_KP_COMMA),
		KP_EQUALSAS400 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_EQUALSAS400),

		ALTERASE = TO_KEYCODE(Key_Scancode::SCANCODE_ALTERASE),
		SYSREQ = TO_KEYCODE(Key_Scancode::SCANCODE_SYSREQ),
		CANCEL = TO_KEYCODE(Key_Scancode::SCANCODE_CANCEL),
		CLEAR = TO_KEYCODE(Key_Scancode::SCANCODE_CLEAR),
		PRIOR = TO_KEYCODE(Key_Scancode::SCANCODE_PRIOR),
		RETURN2 = TO_KEYCODE(Key_Scancode::SCANCODE_RETURN2),
		SEPARATOR = TO_KEYCODE(Key_Scancode::SCANCODE_SEPARATOR),
		OUT = TO_KEYCODE(Key_Scancode::SCANCODE_OUT),
		OPER = TO_KEYCODE(Key_Scancode::SCANCODE_OPER),
		CLEARAGAIN = TO_KEYCODE(Key_Scancode::SCANCODE_CLEARAGAIN),
		CRSEL = TO_KEYCODE(Key_Scancode::SCANCODE_CRSEL),
		EXSEL = TO_KEYCODE(Key_Scancode::SCANCODE_EXSEL),

		KP_00 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_00),
		KP_000 = TO_KEYCODE(Key_Scancode::SCANCODE_KP_000),
		THOUSANDSSEPARATOR = TO_KEYCODE(Key_Scancode::SCANCODE_THOUSANDSSEPARATOR),
		DECIMALSEPARATOR = TO_KEYCODE(Key_Scancode::SCANCODE_DECIMALSEPARATOR),
		CURRENCYUNIT = TO_KEYCODE(Key_Scancode::SCANCODE_CURRENCYUNIT),
		CURRENCYSUBUNIT = TO_KEYCODE(Key_Scancode::SCANCODE_CURRENCYSUBUNIT),
		KP_LEFTPAREN = TO_KEYCODE(Key_Scancode::SCANCODE_KP_LEFTPAREN),
		KP_RIGHTPAREN = TO_KEYCODE(Key_Scancode::SCANCODE_KP_RIGHTPAREN),
		KP_LEFTBRACE = TO_KEYCODE(Key_Scancode::SCANCODE_KP_LEFTBRACE),
		KP_RIGHTBRACE = TO_KEYCODE(Key_Scancode::SCANCODE_KP_RIGHTBRACE),
		KP_TAB = TO_KEYCODE(Key_Scancode::SCANCODE_KP_TAB),
		KP_BACKSPACE = TO_KEYCODE(Key_Scancode::SCANCODE_KP_BACKSPACE),
		KP_A = TO_KEYCODE(Key_Scancode::SCANCODE_KP_A),
		KP_B = TO_KEYCODE(Key_Scancode::SCANCODE_KP_B),
		KP_C = TO_KEYCODE(Key_Scancode::SCANCODE_KP_C),
		KP_D = TO_KEYCODE(Key_Scancode::SCANCODE_KP_D),
		KP_E = TO_KEYCODE(Key_Scancode::SCANCODE_KP_E),
		KP_F = TO_KEYCODE(Key_Scancode::SCANCODE_KP_F),
		KP_XOR = TO_KEYCODE(Key_Scancode::SCANCODE_KP_XOR),
		KP_POWER = TO_KEYCODE(Key_Scancode::SCANCODE_KP_POWER),
		KP_PERCENT = TO_KEYCODE(Key_Scancode::SCANCODE_KP_PERCENT),
		KP_LESS = TO_KEYCODE(Key_Scancode::SCANCODE_KP_LESS),
		KP_GREATER = TO_KEYCODE(Key_Scancode::SCANCODE_KP_GREATER),
		KP_AMPERSAND = TO_KEYCODE(Key_Scancode::SCANCODE_KP_AMPERSAND),
		KP_DBLAMPERSAND = TO_KEYCODE(Key_Scancode::SCANCODE_KP_DBLAMPERSAND),
		KP_VERTICALBAR = TO_KEYCODE(Key_Scancode::SCANCODE_KP_VERTICALBAR),
		KP_DBLVERTICALBAR = TO_KEYCODE(Key_Scancode::SCANCODE_KP_DBLVERTICALBAR),
		KP_COLON = TO_KEYCODE(Key_Scancode::SCANCODE_KP_COLON),
		KP_HASH = TO_KEYCODE(Key_Scancode::SCANCODE_KP_HASH),
		KP_SPACE = TO_KEYCODE(Key_Scancode::SCANCODE_KP_SPACE),
		KP_AT = TO_KEYCODE(Key_Scancode::SCANCODE_KP_AT),
		KP_EXCLAM = TO_KEYCODE(Key_Scancode::SCANCODE_KP_EXCLAM),
		KP_MEMSTORE = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MEMSTORE),
		KP_MEMRECALL = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MEMRECALL),
		KP_MEMCLEAR = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MEMCLEAR),
		KP_MEMADD = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MEMADD),
		KP_MEMSUBTRACT = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MEMSUBTRACT),
		KP_MEMMULTIPLY = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MEMMULTIPLY),
		KP_MEMDIVIDE = TO_KEYCODE(Key_Scancode::SCANCODE_KP_MEMDIVIDE),
		KP_PLUSMINUS = TO_KEYCODE(Key_Scancode::SCANCODE_KP_PLUSMINUS),
		KP_CLEAR = TO_KEYCODE(Key_Scancode::SCANCODE_KP_CLEAR),
		KP_CLEARENTRY = TO_KEYCODE(Key_Scancode::SCANCODE_KP_CLEARENTRY),
		KP_BINARY = TO_KEYCODE(Key_Scancode::SCANCODE_KP_BINARY),
		KP_OCTAL = TO_KEYCODE(Key_Scancode::SCANCODE_KP_OCTAL),
		KP_DECIMAL = TO_KEYCODE(Key_Scancode::SCANCODE_KP_DECIMAL),
		KP_HEXADECIMAL = TO_KEYCODE(Key_Scancode::SCANCODE_KP_HEXADECIMAL),

		LCTRL = TO_KEYCODE(Key_Scancode::SCANCODE_LCTRL),
		LSHIFT = TO_KEYCODE(Key_Scancode::SCANCODE_LSHIFT),
		LALT = TO_KEYCODE(Key_Scancode::SCANCODE_LALT),
		LGUI = TO_KEYCODE(Key_Scancode::SCANCODE_LGUI),
		RCTRL = TO_KEYCODE(Key_Scancode::SCANCODE_RCTRL),
		RSHIFT = TO_KEYCODE(Key_Scancode::SCANCODE_RSHIFT),
		RALT = TO_KEYCODE(Key_Scancode::SCANCODE_RALT),
		RGUI = TO_KEYCODE(Key_Scancode::SCANCODE_RGUI),

		MODE = TO_KEYCODE(Key_Scancode::SCANCODE_MODE),

		AUDIONEXT = TO_KEYCODE(Key_Scancode::SCANCODE_AUDIONEXT),
		AUDIOPREV = TO_KEYCODE(Key_Scancode::SCANCODE_AUDIOPREV),
		AUDIOSTOP = TO_KEYCODE(Key_Scancode::SCANCODE_AUDIOSTOP),
		AUDIOPLAY = TO_KEYCODE(Key_Scancode::SCANCODE_AUDIOPLAY),
		AUDIOMUTE = TO_KEYCODE(Key_Scancode::SCANCODE_AUDIOMUTE),
		MEDIASELECT = TO_KEYCODE(Key_Scancode::SCANCODE_MEDIASELECT),
		WWW = TO_KEYCODE(Key_Scancode::SCANCODE_WWW),
		MAIL = TO_KEYCODE(Key_Scancode::SCANCODE_MAIL),
		CALCULATOR = TO_KEYCODE(Key_Scancode::SCANCODE_CALCULATOR),
		COMPUTER = TO_KEYCODE(Key_Scancode::SCANCODE_COMPUTER),
		AC_SEARCH = TO_KEYCODE(Key_Scancode::SCANCODE_AC_SEARCH),
		AC_HOME = TO_KEYCODE(Key_Scancode::SCANCODE_AC_HOME),
		AC_BACK = TO_KEYCODE(Key_Scancode::SCANCODE_AC_BACK),
		AC_FORWARD = TO_KEYCODE(Key_Scancode::SCANCODE_AC_FORWARD),
		AC_STOP = TO_KEYCODE(Key_Scancode::SCANCODE_AC_STOP),
		AC_REFRESH = TO_KEYCODE(Key_Scancode::SCANCODE_AC_REFRESH),
		AC_BOOKMARKS = TO_KEYCODE(Key_Scancode::SCANCODE_AC_BOOKMARKS),

		BRIGHTNESSDOWN = TO_KEYCODE(Key_Scancode::SCANCODE_BRIGHTNESSDOWN),
		BRIGHTNESSUP = TO_KEYCODE(Key_Scancode::SCANCODE_BRIGHTNESSUP),
		DISPLAYSWITCH = TO_KEYCODE(Key_Scancode::SCANCODE_DISPLAYSWITCH),
		KBDILLUMTOGGLE = TO_KEYCODE(Key_Scancode::SCANCODE_KBDILLUMTOGGLE),
		KBDILLUMDOWN = TO_KEYCODE(Key_Scancode::SCANCODE_KBDILLUMDOWN),
		KBDILLUMUP = TO_KEYCODE(Key_Scancode::SCANCODE_KBDILLUMUP),
		EJECT = TO_KEYCODE(Key_Scancode::SCANCODE_EJECT),
		SLEEP = TO_KEYCODE(Key_Scancode::SCANCODE_SLEEP),
		APP1 = TO_KEYCODE(Key_Scancode::SCANCODE_APP1),
		APP2 = TO_KEYCODE(Key_Scancode::SCANCODE_APP2),

		AUDIOREWIND = TO_KEYCODE(Key_Scancode::SCANCODE_AUDIOREWIND),
		AUDIOFASTFORWARD = TO_KEYCODE(Key_Scancode::SCANCODE_AUDIOFASTFORWARD),

		SOFTLEFT = TO_KEYCODE(Key_Scancode::SCANCODE_SOFTLEFT),
		SOFTRIGHT = TO_KEYCODE(Key_Scancode::SCANCODE_SOFTRIGHT),
		CALL = TO_KEYCODE(Key_Scancode::SCANCODE_CALL),
		ENDCALL = TO_KEYCODE(Key_Scancode::SCANCODE_ENDCALL)
	};

    bool IsKeyDown(Key InputKey);
}

namespace TextInput
{
	extern bool PollForText;
	extern std::string Text;
	extern int TextIndex;
}