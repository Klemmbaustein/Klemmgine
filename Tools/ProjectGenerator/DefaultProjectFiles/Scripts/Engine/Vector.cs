using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct Vector3
{
	public float X, Y, Z;

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

	public float Length()
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

	override public string ToString()
	{
		return string.Format("{0} {1} {2}", X, Y, Z);
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

	private static bool _nearlyequal(float a, float b, float t)
	{
		return a > b - t && a < b + t;
	}

	public static bool NearlyEqual(Vector3 a, Vector3 b, float Threshold)
	{
		return _nearlyequal(a.X, b.X, Threshold) && _nearlyequal(a.Y, b.Y, Threshold) && _nearlyequal(a.Z, b.Z, Threshold);
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
	public Transform(Vector3 x, Vector3 y, Vector3 z)
	{
		Position = x;
		Rotation = y;
		Scale = z;
	}
}