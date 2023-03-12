#pragma once
#include <vector>
#include <string>
#include <glm/fwd.hpp>
#include "glm/ext/vector_float3.hpp"

struct Transform;

struct Vector2
{
	float X = 0;
	float Y = 0;

	Vector2()
	{

	}

	Vector2(float xy)
	{
		X = xy;
		Y = xy;
	}

	Vector2(float x, float y)
	{
		X = x;
		Y = y;
	}

	Vector2& operator+=(Vector2 b);
	Vector2& operator*=(Vector2 b);
	std::string ToString();
	float Length();
	Vector2 Clamp(Vector2 Min, Vector2 Max);
};

/// <summary>
/// A struct holding X, Y and Z floats. Very useful.
/// </summary>
struct Vector3
{
	float X = 0;
	float Y = 0;
	float Z = 0;

	/// <summary>
	/// Constructor initalizing the X, Y and Z values of the Vector indivitually.
	/// </summary>
	/// <param name="x">The new X value</param>
	/// <param name="y">The new Y value</param>
	/// <param name="z">The new Z value</param>
	Vector3(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	/// <summary>
	/// Constructs an emtpy Vector3.
	/// X, Y and Z will be 0.
	/// </summary>
	Vector3() {}

	/// <summary>
	/// Constructor initalizing X, Y and Z to a single Value.
	/// </summary>
	/// <param name="xyz">The value for X, Y and Z.</param>
	Vector3(float xyz)
	{
		X = xyz;
		Y = xyz;
		Z = xyz;
	}

	/// <summary>
	/// Construct a Vector3 from a Vector2 for XY and a float representing Z.
	/// </summary>
	/// <param name="xy">The Vector2 containing the X and Y coordinate</param>
	/// <param name="z">The float for the Z coordinate</param>
	Vector3(Vector2 xy, float z)
	{
		X = xy.X;
		Y = xy.Y;
		Z = z;
	}

	/// <summary>
	/// Copies a glm::vec3 to a Vector3.
	/// </summary>
	/// <param name="xyz">The glm::vec3</param>
	Vector3(glm::vec3 xyz);

	//
	operator glm::vec3()
	{
		return glm::vec3(X, Y, Z);
	}

	float& operator[](int in)
	{
		return at(in);
	}

	Vector3 Normalize();
	float Length();

	Vector3& operator+=(Vector3 a);
	Vector3& operator-=(Vector3 a);
	
	Vector3 operator-();

	Vector3 RadiantsToDegrees();
	Vector3 DegreesToRadiants();
	std::string ToString();

	operator std::string() const
	{
		return std::to_string(X).append(" ").append(std::to_string(Y)).append(" ").append(std::to_string(Z));
	}
	float& at(int Index)
	{
		switch (Index)
		{
		case 0:
			return X;
		case 1:
			return Y;
		case 2:
			return Z;
		default:
			throw "Invalid Vector3 index!";
		}
	}

	static Vector3 Vec3ToVector(glm::vec3 In);
	static Vector3 GetForwardVector(Vector3 In);
	static Vector3 GetRightVector(Vector3 In);
	static Vector3 GetUpVector(Vector3 In);
	static Vector3 Cross(Vector3 a, Vector3 b);
	static Vector3 stov(std::string In);
	static Vector3 SnapToGrid(Vector3 In, float GridSize);
	static Vector3 Lerp(Vector3 a, Vector3 b, float val);
	static float Dot(Vector3 a, Vector3 b);
	// LookAtFunction, but instead of the rotation assuming Z is forward, Y is.
	static Vector3 LookAtFunctionY(Vector3 Start, Vector3 End, bool Radiants = false);
	static Vector3 LookAtFunction(Vector3 Start, Vector3 End, bool Radiants = false);
	static Vector3 QuatToEuler(glm::quat quat);
	static float Distance(Vector3 a, Vector3 b);
	static Vector3 GetScaledAxis(Vector3 Rot, unsigned int Dir);
	static Vector3 RotateVector(Vector3 Vec, Vector3 Rot);
	static Vector3 TranslateVector(Vector3 Vec, Transform Transform);
	static bool NearlyEqual(Vector3 a, Vector3 b, float Threshold);
};

Vector3 operator+(Vector3 a, Vector3 b);

Vector3 operator-(Vector3 a, Vector3 b);

Vector3 operator*(Vector3 a, Vector3 b);

Vector3 operator*(Vector3 a, float b);

Vector3 operator/(Vector3 a, Vector3 b);

Vector2 operator+(Vector2 a, Vector2 b);

Vector2 operator-(Vector2 a, Vector2 b);

Vector2 operator*(Vector2 a, float b);

Vector2 operator/(Vector2 a, Vector2 b);

Vector2 operator/(Vector2 a, float b);

bool operator==(Vector2 a, float b);

inline bool operator!=(Vector2 a, float b)
{
	return !(a == b);
}
bool operator==(Vector2 a, Vector2 b);
bool operator==(Vector3 a, Vector3 b);
inline bool operator!=(Vector3 a, Vector3 b)
{
	return !(a == b);
}
inline bool operator!=(Vector2 a, Vector2 b)
{
	return !(a == b);
}

struct Transform
{
	Vector3 Location = Vector3(0.f);
	Vector3 Rotation = Vector3(0.f);
	Vector3 Scale = Vector3(1.f);

	Transform(Vector3 Loc, Vector3 Rot, Vector3 Scl)
	{
		Location = Loc;
		Rotation = Rot;
		Scale = Scl;
	}
	Transform()
	{

	}

	glm::mat4 ToMatrix();
};

bool operator==(Transform a, Transform b);
inline bool operator!=(Transform a, Transform b)
{
	return !(a == b);
}

Transform operator+(Transform a, Transform b);

struct Vector4
{
	float X = 0;
	float Y = 0;
	float Z = 0;
	float W = 0;

	Vector4(float XYZW)
	{
		X = XYZW;
		Y = XYZW;
		Z = XYZW;
		W = XYZW;
	}

	Vector4(Vector2 XY, Vector2 ZW)
	{
		X = XY.X;
		Y = XY.Y;
		Z = ZW.X;
		W = ZW.Y;
	}

	Vector4()
	{

	}
	Vector4(float X, float Y, float Z, float W)
	{
		this->X = X;
		this->Y = Y;
		this->Z = Z;
		this->W = W;
	}

	Vector4(Vector3 XYZ, float W)
	{
		X = XYZ.X;
		Y = XYZ.Y;
		Z = XYZ.Z;
		this->W = W;
	}
};
