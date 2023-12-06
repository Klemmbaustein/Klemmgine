using System;
using System.Runtime.InteropServices;
using System.Xml.Linq;

namespace UI
{
	public class UIBox
	{
		protected delegate IntPtr CreateBox(bool Horizontal, Vector2 Position);
		protected delegate void DestroyBox(IntPtr Box);
		protected delegate void SetBoxVec2(Vector2 Val, IntPtr Box);
		protected delegate void AddChildDelegate(IntPtr Child, IntPtr Box);
		protected delegate void SetAlignDelegate(Align NewAlign, IntPtr Box);
		protected delegate void SetBorderDelegate(BorderType NewBorder, float Size, IntPtr Box);
		protected delegate void SetPaddingDelegate(float UpPadding, float DownPadding, float LeftPadding, float RightPadding, IntPtr Box);
		protected delegate void SetSizeModeDelegate(SizeMode mode, IntPtr Box);
		protected IntPtr NativePtr;

		public enum Orientation
		{
			Horizontal,
			Vertical
		}

		public enum Align
		{
			Default,
			Centered,
			Reverse
		}

		public enum BorderType
		{
			None,
			Rounded,
			DarkenedEdge
		}
		
		public enum SizeMode
		{
			ScreenRelative,
			PixelRelative
		}

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
		}

		public UIBox SetMinSize(Vector2 NewMinSize)
		{
			NativeFunction.CallNativeFunction("SetUIBoxMinSize",
				typeof(SetBoxVec2),
				new object[] {NewMinSize, NativePtr});
			return this;
		}

		public UIBox SetMaxSize(Vector2 NewMaxSize)
		{
			NativeFunction.CallNativeFunction("SetUIBoxMaxSize",
				typeof(SetBoxVec2),
				new object[] { NewMaxSize, NativePtr });
			return this;
		}

		public UIBox SetPosition(Vector2 NewPosition)
		{
			NativeFunction.CallNativeFunction("SetUIBoxPosition",
				typeof(SetBoxVec2),
				new object[] { NewPosition, NativePtr });
			return this;
		}

		public UIBox AddChild(UIBox Child)
		{
			NativeFunction.CallNativeFunction("AddUIBoxChild",
				typeof(AddChildDelegate),
				new object[] {Child.NativePtr, NativePtr});
			return this;
		}

		public UIBox SetHorizontalAlign(Align NewAlign)
		{
			NativeFunction.CallNativeFunction("SetUIBoxHorizontalAlign",
				typeof(SetAlignDelegate),
				new object[] { NewAlign, NativePtr });

			return this;
		}

		public UIBox SetVerticalAlign(Align NewAlign)
		{
			NativeFunction.CallNativeFunction("SetUIBoxVerticalAlign",
				typeof(SetAlignDelegate),
				new object[] { NewAlign, NativePtr });

			return this;
		}

		public UIBox SetSizeMode(SizeMode Mode)
		{
			NativeFunction.CallNativeFunction("SetUIBoxSizeMode",
				typeof(SetSizeModeDelegate),
				new object[] { Mode, NativePtr });

			return this;
		}

		public UIBox SetBorder(BorderType NewBorderType, float Size)
		{
			NativeFunction.CallNativeFunction("SetUIBoxBorder",
				typeof(SetBorderDelegate),
				new object[] { NewBorderType, Size, NativePtr });

			return this;
		}

		public UIBox SetPadding(float Padding)
		{
			SetPadding(Padding, Padding, Padding, Padding);
			return this;
		}

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

	public class UIBackground : UIBox
	{
		protected delegate IntPtr CreateBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale);

		public UIBackground(Orientation BoxOritentation, Vector2 Position, Vector3 Color, Vector2 MinScale) 
			: base(BoxOritentation, Position)
		{
			NativePtr = (IntPtr)NativeFunction.CallNativeFunction("CreateUIBackground",
				typeof(CreateBackground),
				new object[]
				{
					BoxOritentation == Orientation.Horizontal,
					Position,
					Color,
					MinScale
				});
		}
	}

	public class UIText : UIBox
	{
		protected delegate IntPtr CreateText(float Scale, Vector3 Color, [MarshalAs(UnmanagedType.LPUTF8Str)] string Text, IntPtr TextFont);
		Font UsedFont = null;

		public class Font
		{
			protected delegate IntPtr CreateFont([MarshalAs(UnmanagedType.LPUTF8Str)] string Name);

			public Font([MarshalAs(UnmanagedType.LPUTF8Str)] string FontName = "Font.ttf")
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

		public UIText(float Scale, Vector3 Color, string Text, Font TextFont)
			: base(UIBox.Orientation.Horizontal, 0)
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