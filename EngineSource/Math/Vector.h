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
	Vector2 operator*(Vector2 b);
	std::string ToString();
	float Length();
	Vector2 Clamp(Vector2 Min, Vector2 Max);
};

/// <summary>
/// A struct holding X, Y and Z floats.
/// </summary>
struct Vector3
{
	float X = 0;
	float Y = 0;
	float Z = 0;

	Vector3(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	Vector3() {}

	Vector3(float xyz)
	{
		X = xyz;
		Y = xyz;
		Z = xyz;
	}

	Vector3(Vector2 xy, float z)
	{
		X = xy.X;
		Y = xy.Y;
		Z = z;
	}

	Vector3(glm::vec3 xyz);

	operator glm::vec3()
	{
		return glm::vec3(X, Y, Z);
	}

	float& operator[](int in)
	{
		return at(in);
	}

	Vector3 Normalize() const;
	float Length() const;

	Vector3& operator+=(Vector3 a);
	Vector3& operator-=(Vector3 a);
	
	Vector3 operator-();

	Vector3 RadiantsToDegrees() const;
	Vector3 DegreesToRadiants() const;
	std::string ToString() const;

	operator std::string() const
	{
		return std::to_string(X).append(" ").append(std::to_string(Y)).append(" ").append(std::to_string(Z));
	}
	float& at(unsigned int Index);

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
	static Vector3 Clamp(Vector3 v, Vector3 min, Vector3 max);
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
