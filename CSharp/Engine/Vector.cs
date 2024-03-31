using System;
using System.Globalization;
using System.Runtime.InteropServices;
using Engine.Native;

namespace Engine;

/**
 * @ingroup CSharp
 * @brief
 * A structure containing X, Y and Z coordinates.
 * 
 * Equivalent to 'struct Vector3' in 'Math/Vector.h'
 */
[StructLayout(LayoutKind.Sequential)]
public struct Vector3
{
	public float X, Y, Z;

	private delegate Vector3 VecScaledAxis(Vector3 Rotation, uint Dir);
	public Vector3()
	{
		X = 0;
		Y = 0;
		Z = 0;
	}
	public Vector3(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}
	public Vector3(float xyz)
	{
		X = xyz;
		Y = xyz;
		Z = xyz;
	}

	/**
	 */
	public readonly float Length()
	{
		return MathF.Sqrt(X * X + Y * Y + Z * Z);
	}

	public Vector3 Normalized()
	{
		float Len = Length();
		if (Len != 0)
		{
			return this / Len;
		}
		return 0;
	}

	public override readonly string ToString()
	{
		var Culture = CultureInfo.GetCultureInfo("en-US");
		return string.Format("{0} {1} {2}",
			X.ToString("f", Culture),
			Y.ToString("f", Culture),
			Z.ToString("f", Culture));
	}

	public static Vector3 operator+(Vector3 val)
	{
		return new Vector3(+val.X, +val.Y, +val.Z);
	}
	public static Vector3 operator-(Vector3 val)
	{
		return new Vector3(-val.X, -val.Y, -val.Z);
	}
	public static Vector3 operator+(Vector3 a, Vector3 b)
	{
		return new Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
	}
	public static Vector3 operator-(Vector3 a, Vector3 b)
	{
		return new Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
	}
	public static Vector3 operator*(Vector3 a, Vector3 b)
	{
		return new Vector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
	}
	public static Vector3 operator/(Vector3 a, Vector3 b)
	{
		return new Vector3(a.X / b.X, a.Y / b.Y, a.Z / b.Z);
	}

	public static implicit operator Vector3(float val)
	{
		return new Vector3(val, val, val);
	}

	private static bool NearlyEqual(float a, float b, float t)
	{
		return a > b - t && a < b + t;
	}

	public enum Axis
	{
		X = 0,
		Y = 1,
		Z = 2
	}

	public static Vector3 GetScaledAxis(Vector3 Rot, Axis Dir)
	{
		return (Vector3)NativeFunction.CallNativeFunction(
			"Vector3::GetScaledAxis",
			typeof(VecScaledAxis),
			new object[] 
			{ 
				Rot / 180 * MathF.PI,
				(UInt32)Dir 
			});
	}

	public static Vector3 GetForwardVector(Vector3 Rotation)
	{
		return GetScaledAxis(Rotation, Axis.Z);
	}

	public static Vector3 GetRightVector(Vector3 Rotation)
	{
		return GetScaledAxis(Rotation, Axis.X);
	}

	public static Vector3 GetUpVector(Vector3 Rotation)
	{
		return GetScaledAxis(Rotation, Axis.Y);
	}

	public static bool NearlyEqual(Vector3 a, Vector3 b, float Threshold)
	{
		return NearlyEqual(a.X, b.X, Threshold) && NearlyEqual(a.Y, b.Y, Threshold) && NearlyEqual(a.Z, b.Z, Threshold);
	}
}

/**
 * @brief
 * A structure containing single precision X and Y coordinates.
 * 
 * Equivalent to 'struct Vector2' in 'Math/Vector.h'
 */
[StructLayout(LayoutKind.Sequential)]
public struct Vector2
{
	public float X = 0;
	public float Y = 0;

	public Vector2()
	{

	}

	public Vector2(float X, float Y)
	{
		this.X = X;
		this.Y = Y;
	}

	public Vector2(float XY)
	{
		X = XY;
		Y = XY;
	}

	public static implicit operator Vector2(float val)
	{
		return new Vector2(val, val);
	}

	public override readonly string ToString()
	{
		var Culture = CultureInfo.GetCultureInfo("en-US");
		return string.Format("{0} {1}",
			X.ToString("f", Culture),
			Y.ToString("f", Culture));
	}
	public static Vector2 operator +(Vector2 a, Vector2 b)
	{
		return new Vector2(a.X + b.X, a.Y + b.Y);
	}
	public static Vector2 operator -(Vector2 a, Vector2 b)
	{
		return new Vector2(a.X - b.X, a.Y - b.Y);
	}
	public static Vector2 operator *(Vector2 a, Vector2 b)
	{
		return new Vector2(a.X * b.X, a.Y * b.Y);
	}
	public static Vector2 operator /(Vector2 a, Vector2 b)
	{
		return new Vector2(a.X / b.X, a.Y / b.Y);
	}
}


[StructLayout(LayoutKind.Sequential)]
public struct Transform
{
	public Vector3 Position;
	public Vector3 Rotation;
	public Vector3 Scale;

	public Transform()
	{
		Position = new Vector3();
		Rotation = new Vector3();
		Scale = new Vector3(1);
	}
	public Transform(Vector3 position, Vector3 rotation, Vector3 scale)
	{
		Position = position;
		Rotation = rotation;
		Scale = scale;
	}
}