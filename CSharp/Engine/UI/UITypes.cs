using System;
using System.Runtime.InteropServices;


/**
 * @defgroup CSharp-UI
 * @ingroup CSharp
 * 
 * @brief
 * C# API for UI.
 * 
 * ## Major differences between C++ and C#:
 * - The TextRenderer class in C++ is called UIText.Font in C#.
 */

/**
 * @ingroup CSharp
 * @brief
 * Namespace containing C# UI functions.
 * 
 * No equivalent C++ namespace exists.
 */
namespace Engine.UI
{
	/** 
	 * @brief
	 * C# UIBox class. Parent class for all UI elements.
	 * 
	 * More information in C++ UIBox documentation. Most of it also applies for C#.
	 * 
	 * 
	 * @ingroup CSharp-UI
	 */
	public class UIBox
	{
		protected delegate IntPtr CreateBox(bool Horizontal, Vector2 Position);
		protected delegate void DestroyBox(IntPtr Box);
		protected delegate void SetBoxVec2(Vector2 Val, IntPtr Box);
		protected delegate void AddChildDelegate(IntPtr Child, IntPtr Box);
		protected delegate void SetAlignDelegate(Align NewAlign, IntPtr Box);
		protected delegate void SetBorderDelegate(BorderType NewBorder, float Size, IntPtr Box);
		protected delegate void SetPaddingDelegate(float UpPadding, float DownPadding, float LeftPadding, float RightPadding, IntPtr Box);
		protected delegate void SetSizeModeDelegate(SizeMode Mode, IntPtr Box);
		protected delegate Vector3 GetVector2(IntPtr NativePtr);
		protected IntPtr NativePtr;

		/**
		 * @brief
		 * Controls the orientation for all children.
		 * 
		 * If Orientation is Horizontal, the children of this UIBox will be aligned horizontally.
		 * 
		 * ```
		 * Example:
		 * 
		 *  ____________________________________    ____________________________________
		 * | _______   _______                  |  | _______                            |
		 * ||Child 1| |Child 2|                 |  ||Child 1|                           |
		 * ||_______| |_______|                 |  ||_______|  Parent box               |
		 * |                                    |  | _______   Orientation: Vertical    |
		 * |      Parent box                    |  ||Child 2|                           |
		 * |      Orientation: Horizontal       |  ||_______|                           |
		 * |____________________________________|  |____________________________________|
		 * ```
		 */
		public enum Orientation
		{
			/// Vertical orientation for all children.
			Horizontal,
			/// Horizontal orientation for all children.
			Vertical
		}

		/**
		 * @brief
		 * Children align for UIBox.
		 */
		public enum Align
		{
			/// Align from lowest to highest. if Orientation = Horizontal, this means from left to right.
			Default,
			/// Centered align. will act like Align.Default, but children will be put in the center of the box.
			Centered,
			/// Align from highest to lowest. if Orientation = Horizontal, this means from right to left.
			Reverse
		}

		/**
		 * @brief
		 * Describes the border of an Engine.UI.UIBox.
		*/
		public enum BorderType
		{
			/// No border.
			None,
			/// Rounded border.
			Rounded,
			/// Darkened edge around the box.
			DarkenedEdge
		}
		
		/**
		 * @brief
		 * Describes the way size should be calculated for a UIBox.
		 */
		public enum SizeMode
		{
			/**
			 * @brief
			 * Default value. Size should be relative to the screen.
			 * 
			 * A box with the position x=-1, y=-1 (bottom left corner) and a size of x=1, y=2 will always fill half the screen.
			 * 
			 * A box where `size x` = `size y` will only be square if the screen itself is square.
			 */
			ScreenRelative,

			/**
			 * @brief
			 * Size should be relative to the aspect ratio/pixels.
			 * 
			 * A box where `size x` = `size y` is guaranteed to be square.
			 * 
			 * A PixelRelative box with the size x=1, x=1 has the same size as a ScreenRelative box with the size x=1/AspectRatio, y=1.
			 */
			PixelRelative
		}

		/**
		 * @brief
		 * Constructs UIBox with the given orientation and position.
		 */
		public UIBox(Orientation BoxOritentation, Vector2 Position)
		{
			if (GetType() == typeof(UIBox))
			{
				NativePtr = (IntPtr)NativeFunction.CallNativeFunction("CreateUIBox",
					typeof(CreateBox),
					new object[]
					{
					BoxOritentation == Orientation.Horizontal,
					Position
					});
			}
		}

		public void Destroy()
		{
			NativeFunction.CallNativeFunction("DestroyUIBox",
				typeof(DestroyBox),
				new object[]
				{
					NativePtr
				});
			NativePtr = IntPtr.Zero;
		}

		/**
		 * @brief
		 * Sets the minimum size this UIBox can occupy.
		 * 
		 * C++ equivalent is UIBox::SetMinSize()
		 * 
		 * @param NewMinSize
		 * The new minimum size the UIBox should occupy.
		 * 
		 * @return
		 * A reference to this %UIBox.
		 */
		public UIBox SetMinSize(Vector2 NewMinSize)
		{
			NativeFunction.CallNativeFunction("SetUIBoxMinSize",
				typeof(SetBoxVec2),
				new object[] {NewMinSize, NativePtr});
			return this;
		}

		/**
		 * @brief
		 * Sets the maximum size this UIBox can occupy.
		 * 
		 * C++ equivalent is UIBox::SetMaxSize()
		 * 
		 * @param NewMaxSize
		 * The new maximum size the UIBox should occupy.
		 * 
		 * @return
		 * A reference to this %UIBox.
		 */
		public UIBox SetMaxSize(Vector2 NewMaxSize)
		{
			NativeFunction.CallNativeFunction("SetUIBoxMaxSize",
				typeof(SetBoxVec2),
				new object[] { NewMaxSize, NativePtr });
			return this;
		}

		/**
		 * @brief
		 * Gets the used size of the UIBox, the size that the UIBox occupies.
		 * 
		 * @return
		 * The used size of the box.
		 */
		public Vector2 GetUsedSize()
		{
			var vec3 = (Vector3)NativeFunction.CallNativeFunction("GetUIBoxSize",
				typeof(GetVector2),
				new object[] { NativePtr });
			return new Vector2(vec3.X, vec3.Y);

		}

		/**
		 * @brief
		 * Gets the position of the UIBox.
		 * 
		 * @return
		 * The position of the box, where -1, -1 is the bottom left corner of the screen and 1, 1 is the top right corner.
		 */
		public Vector2 GetPosition()
		{
			var vec3 = (Vector3)NativeFunction.CallNativeFunction("GetUIBoxPosition",
				typeof(GetVector2),
				new object[] { NativePtr });
			return new Vector2(vec3.X, vec3.Y);
		}

		/**
		 * @brief
		 * Sets the position of the UIBox.
		 * 
		 * @param NewPosition
		 * The new position of the box, where -1, -1 is the bottom left corner of the screen and 1, 1 is the top right corner.
		 */
		public UIBox SetPosition(Vector2 NewPosition)
		{
			NativeFunction.CallNativeFunction("SetUIBoxPosition",
				typeof(SetBoxVec2),
				new object[] { NewPosition, NativePtr });
			return this;
		}

		/**
		 * @brief
		 * Adds a child to the %UIBox.
		 * 
		 * The child will follow the orientation and align of the parent.
		 */
		public UIBox AddChild(UIBox Child)
		{
			NativeFunction.CallNativeFunction("AddUIBoxChild",
				typeof(AddChildDelegate),
				new object[] {Child.NativePtr, NativePtr});
			return this;
		}

		/**
		 * @brief
		 * Sets the horizontal align for all children.
		 * 
		 * ```
		 * Example
		 *  ____________________________________    ____________________________________
		 * | _______   _______                  |  |         _______   _______          |
		 * ||Child 1| |Child 2|                 |  |        |Child 1| |Child 2|         |
		 * ||_______| |_______|                 |  |        |_______| |_______|         |
		 * |                                    |  |                                    |
		 * | Parent box                         |  | Parent box                         |
		 * | Horizontal Align: Default          |  | Horizontal Align: Centered         |
		 * |____________________________________|  |____________________________________|
		 * 
		 * @return
		 * A reference to this %UIBox.
		 * ```
		 */
		public UIBox SetHorizontalAlign(Align NewAlign)
		{
			NativeFunction.CallNativeFunction("SetUIBoxHorizontalAlign",
				typeof(SetAlignDelegate),
				new object[] { NewAlign, NativePtr });

			return this;
		}

		/**
		 * @brief
		 * Sets the horizontal align for all children.
		 * 
		 * Notes:
		 * - The default vertical align is Align.Reverse, not Align.Default.
		 * - Align.Reverse aligns boxes from the top down (from 1 to -1) while Align.Default aligns boxes from the bottom up (from -1 to 1)
		 *   The reason for this is that this corresponds to the way horizotal align works.
		 *   (Default is from -1 to 1 - left to right, Reverse 1 to -1, right to left)
		 * 
		 * ```
		 * Example
		 *  ____________________________________    ____________________________________
		 * | _______   _______                  |  | Vertical Align: Default            |
		 * ||Child 1| |Child 2|                 |  | Parent box                         |
		 * ||_______| |_______|                 |  |                                    |
		 * |                                    |  | _______   _______                  |
		 * | Parent box                         |  ||Child 1| |Child 2|                 |
		 * | Vertical Align: Reverse            |  ||_______| |_______|                 |
		 * |____________________________________|  |____________________________________|
		 * ```
		 * @return
		 * A reference to this %UIBox.
		 */
		public UIBox SetVerticalAlign(Align NewAlign)
		{
			NativeFunction.CallNativeFunction("SetUIBoxVerticalAlign",
				typeof(SetAlignDelegate),
				new object[] { NewAlign, NativePtr });

			return this;
		}

		/**
		 * @brief
		 * Sets the size mode of the box.
		 * 
		 * See Engine.UI.UIBox.SizeMode for more info about size modes.
		 * 
		 * @param Mode
		 * The new size mode.
		 * 
		 * @return
		 * A reference to this %UIBox.
		 */
		public UIBox SetSizeMode(SizeMode Mode)
		{
			NativeFunction.CallNativeFunction("SetUIBoxSizeMode",
				typeof(SetSizeModeDelegate),
				new object[] { Mode, NativePtr });

			return this;
		}

		/**
		 * @brief
		 * Sets the border type of the UIBox.
		 * 
		 * This only has an effect on the classes UIButton, UIBackground and UITextField.
		 * 
		 * @param NewBorderType
		 * The type of the border. See Engine.UI.UIBox.BorderType for more info.
		 * 
		 * @param Size
		 * The size of the border.
		 * 
		 * @return
		 * A reference to this %UIBox.
		 */
		public UIBox SetBorder(BorderType NewBorderType, float Size)
		{
			NativeFunction.CallNativeFunction("SetUIBoxBorder",
				typeof(SetBorderDelegate),
				new object[] { NewBorderType, Size, NativePtr });

			return this;
		}

		/**
		 * @brief
		 * Sets the padding of a UIBox, in all directions.
		 * 
		 * Padding works like margin in CSS.
		 * 
		 * @return
		 * A reference to this %UIBox.
		 */
		public UIBox SetPadding(float Padding)
		{
			SetPadding(Padding, Padding, Padding, Padding);
			return this;
		}

		/**
		 * @brief
		 * Sets the padding of a UIBox in each direction.
		 * 
		 * Padding works like margin in CSS.
		 * 
		 * @return
		 * A reference to this %UIBox.
		 */
		public UIBox SetPadding(float UpPadding, float DownPadding, float LeftPadding, float RightPadding)
		{
			NativeFunction.CallNativeFunction("SetUIBoxPadding",
				typeof(SetPaddingDelegate),
				new object[] { UpPadding, DownPadding, LeftPadding, RightPadding, NativePtr });

			return this;
		}

		public UIBox SetPaddingSizeMode(SizeMode NewSizeMode)
		{
			NativeFunction.CallNativeFunction("SetUIBoxPaddingSizeMode",
				typeof(SetSizeModeDelegate),
				new object[] { NewSizeMode, NativePtr });

			return this;
		}
	}

	/**
	 * @brief
	 * UI element that draws a square over the space it occupies.
	 * 
	 * Equivalent to the UIBackground class in C++.
	 * 
	 * The UIBackground class can have a color, opacity and texture.
	 * 
	 * @todo Opacity functions for C#.
	 * @ingroup CSharp-UI
	 */
	public class UIBackground : UIBox
	{
		protected delegate IntPtr CreateBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale);
		protected delegate void SetBackgroundColor(IntPtr Target, Vector3 NewColor);
		protected delegate Vector3 GetBackgroundColor(IntPtr Target);
		protected delegate void SetTextureDelegate(IntPtr Target, [MarshalAs(UnmanagedType.LPUTF8Str)] string Name);


		/**
		 * @brief
		 * Constructs a UIBackground.
		 * 
		 * @param BoxOrientation
		 * The child orientation of the background. If the orientation is Orientation.Horizontal, all children will be
		 * laid out horizontally, if it is Orientation.Vertical, all children will be laid out vetically.
		 * 
		 * @param Position
		 * The position of the background, where -1, -1 is the bottom left corner of the screen and 1, 1 is the top right corner.
		 * 
		 * @param Color
		 * The color of the background, in rgb from 0 - 1.
		 * 
		 * @param MinScale
		 * The minimum scale the background should occupy.
		 */
		public UIBackground(Orientation BoxOrientation, Vector2 Position, Vector3 Color, Vector2 MinScale) 
			: base(BoxOrientation, Position)
		{
			NativePtr = (IntPtr)NativeFunction.CallNativeFunction("CreateUIBackground",
				typeof(CreateBackground),
				new object[]
				{
					BoxOrientation == Orientation.Horizontal,
					Position,
					Color,
					MinScale
				});
		}

		/**
		 * @brief
		 * Sets the color of the background to the given color.
		 * 
		 * A UI redraw will be triggered if the new color is different than the old one.
		 * If there is no difference in color, the cost of calling this function is minimal.
		 * 
		 * @param NewColor
		 * The new color the UIBackground should have.
		 * 
		 * @return
		 * A reference to this UIBackground.
		 */
		public UIBackground SetColor(Vector3 NewColor)
		{
			NativeFunction.CallNativeFunction("SetUIBackgroundColor",
				typeof(SetBackgroundColor),
				new object[] { NativePtr, NewColor });
			return this;
		}

		/**
		 * @brief
		 * Gets the color of the background.
		 * 
		 * @return
		 * The color of this background.
		 */
		public Vector3 GetColor()
		{
			return (Vector3)NativeFunction.CallNativeFunction("GetUIBackgroundColor",
				typeof(GetBackgroundColor),
				new object[] { NativePtr });
		}

		/**
		 * Sets the texture of the UIBackground.
		 * 
		 * If the TextureName is empty, no texture will be used.
		 * 
		 * @param TextureName
		 * The name of the texture file. (without a path or extension)
		 * 
		 * @return
		 * A reference to this UIBackground.
		 */
		public UIBackground SetTexture(string TextureName)
		{
			NativeFunction.CallNativeFunction("SetUIBackgroundTexture",
				typeof(SetTextureDelegate),
				new object[] { NativePtr, TextureName });
			return this;
		}

	}

	/**
	 * @brief
	 * UI element that displays a Text string with the given Engine.UI.UIText.Font.
	 * 
	 * Equivalent to the UIText class in C++.
	 * 
	 * @ingroup CSharp-UI
	 */
	public class UIText : UIBox
	{
		protected delegate IntPtr CreateText(float Scale, Vector3 Color, [MarshalAs(UnmanagedType.LPUTF8Str)] string Text, IntPtr TextFont);
		protected delegate IntPtr SetTextDelegate([MarshalAs(UnmanagedType.LPUTF8Str)] string Text, IntPtr NativePtr);
		protected delegate IntPtr SetColorDelegate(Vector3 Color, IntPtr NativePtr);
		readonly Font UsedFont = null;

		/**
		 * @brief
		 * Equivalent to TextRenderer in C++. Contains information about a font, loaded from a .ttf file.
		 * 
		 * @ingroup CSharp-UI
		 */
		public class Font
		{
			protected delegate IntPtr CreateFont([MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

			/**
			 * @brief
			 * Loads a font.
			 * 
			 * @param FontName
			 * Name of the font file. By default, it will search for fonts in the `{ProjectPath}/Fonts/` directory.
			 */
			public Font(string FontName = "Font.ttf")
			{
				NativePtr = (IntPtr)NativeFunction.CallNativeFunction("CreateTextRenderer",
					typeof(CreateFont),
					new object[]
					{
						FontName
					});
			}

			public IntPtr NativePtr;
		}

		public UIText SetText(string NewText)
		{
			NativeFunction.CallNativeFunction("SetUITextText",
				typeof(SetTextDelegate),
				new object[]
				{
					NewText,
					NativePtr
				});
			return this;
		}

		public UIText SetColor(Vector3 Color)
		{
			NativeFunction.CallNativeFunction("SetUITextColor",
				typeof(SetColorDelegate),
				new object[]
				{
					Color,
					NativePtr
				});
			return this;
		}

		public Font GetFont()
		{
			return UsedFont;
		}

		public UIText(float Scale, Vector3 Color, string Text, Font TextFont)
			: base(Orientation.Horizontal, 0)
		{
			UsedFont = TextFont;

			NativePtr = (IntPtr)NativeFunction.CallNativeFunction("CreateUIText",
				typeof(CreateText),
				new object[]
				{
					Scale,
					Color,
					Text,
					TextFont.NativePtr
				});
		}
	}
}