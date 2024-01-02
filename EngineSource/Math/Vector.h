#pragma once
#include <vector>
#include <string>
#include <glm/fwd.hpp>
#include "glm/ext/vector_float3.hpp"

struct Transform;

/**
* @file
*/

/**
* @brief
* A struct containing an X and Y value. Can be used to represent a point in space.
*/
struct Vector2
{
	/// X coordinate. Right.
	float X = 0;
	/// Y coordinate. Up.
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

	/**
	* @brief 
	* Returns a string representation of the Vector2.
	* 
	* @return
	* A string with the format "{X} {Y}"
	*/
	std::string ToString() const;

	/**
	* @brief
	* Returns the length of the Vector2.
	* 
	* @return sqrt(X^2 + Y^2)
	*/
	float Length() const;

	/**
	* @brief
	* Clamps the value between Min and Max.
	*/
	Vector2 Clamp(Vector2 Min, Vector2 Max) const;
};

/**
* @brief
* A struct containing an X, Y and Z value. Can be used to represent a point in space.
*/
struct Vector3
{
	/// X coordinate. Right.
	float X = 0;
	/// Y coordinate. Up.
	float Y = 0;
	/// Z coordinate. Forward.
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

	operator glm::vec3() const
	{
		return glm::vec3(X, Y, Z);
	}

	float& operator[](int in)
	{
		return at(in);
	}

	/**
	* @brief
	* Calculates a normalized vector.
	* 
	* @return
	* If Length() is greater than 0, all members of the struct will be divided by Length(). Otherwise the return value is Vector3(0).
	*/
	Vector3 Normalize() const;
	
	/**
	* @brief
	* Returns the length of the Vector3.
	* 
	* @return sqrt(X^2 + Y^2 + Z^2)
	*/
	float Length() const;

	Vector3& operator+=(Vector3 a);
	Vector3& operator-=(Vector3 a);
	
	Vector3 operator-();

	Vector3 RadiantsToDegrees() const;
	Vector3 DegreesToRadiants() const;

	/**
	* @brief 
	* Returns a string representation of the Vector3.
	* 
	* @return
	* A string with the format "{X} {Y} {Z}"
	*/
	std::string ToString() const;

	operator std::string() const
	{
		return std::to_string(X).append(" ").append(std::to_string(Y)).append(" ").append(std::to_string(Z));
	}

	/**
	* @brief
	* Returns the value at the Index.
	*/
	float& at(unsigned int Index);

	/**
	* @brief
	* Returns the absolute value of the vector.
	*/
	Vector3 Abs();

	static Vector3 GetForwardVector(Vector3 In);
	static Vector3 GetRightVector(Vector3 In);
	static Vector3 GetUpVector(Vector3 In);
	static Vector3 Cross(Vector3 a, Vector3 b);

	/**
	* @brief
	* Constructs a vector from the given string.
	* 
	* The string must follow the format {X} {Y} {Z}.
	* If the function fails, an empty vector (0, 0, 0) is returned.
	*/
	static Vector3 FromString(std::string In);
	static Vector3 SnapToGrid(Vector3 In, float GridSize);

	/**
	* @brief
	* Linearly interpolates between point a and point b.
	*/
	static Vector3 Lerp(Vector3 a, Vector3 b, float val);

	/**
	* @brief
	* Calculates the dot product from vectors a and b.
	*/
	static float Dot(Vector3 a, Vector3 b);

	/**
	* @brief
	* Calculates a rotation pointing from Start to the point End.
	*
	* Like LookAtFunction, but instead of the rotation assuming Z is forward, Y is.
	* 
	* @param Start
	* The point from which the look at rotation is calculated.
	*
	* @param End
	* The look-at target
	*
	* @param Radiants
	* If true, the rotation will be given in radiants. If false, the rotation will be in degrees.
	*
	* @return
	* A rotation pointing from Start to End, where Y is forward. If Radiants is true, this rotation will be in radiants, otherwise it will be in degrees.
	*/
	static Vector3 LookAtFunctionY(Vector3 Start, Vector3 End, bool Radiants = false);

	/**
	* @brief
	* Calculates a rotation pointing from Start to the point End.
	* 
	* @param Start
	* The point from which the look at rotation is calculated.
	* 
	* @param End
	* The look-at target
	* 
	* @param Radiants
	* If true, the rotation will be given in radiants. If false, the rotation will be in degrees.
	* 
	* @return
	* A rotation pointing from Start to End. If Radiants is true, this rotation will be in radiants, otherwise it will be in degrees.
	*/
	static Vector3 LookAtFunction(Vector3 Start, Vector3 End, bool Radiants = false);
	static Vector3 QuatToEuler(glm::quat quat);

	/**
	* @brief
	* Returns the distance between point a and b.
	*/
	static float Distance(Vector3 a, Vector3 b);
	static Vector3 GetScaledAxis(Vector3 Rot, unsigned int Dir);
	static Vector3 RotateVector(Vector3 Vec, Vector3 Rot);

	/**
	* @brief
	* Applies the given to the Vector3 Vec.
	* 
	* @param Vec
	* The Vector3 to apply the Transform to.
	* 
	* @param t
	* The Transform that should be applied to vec.
	* 
	* @return
	* Returns Vec, but translated by the transform t.
	*/
	static Vector3 TranslateVector(Vector3 Vec, Transform t);

	/**
	* @brief
	* Returns true if the difference between a and b is less than Treshold.
	*/
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

/**
* @brief
* A struct containing Location, Rotation and Scale in a 3d space.
*/
struct Transform
{
	/// Location of the Transform.
	Vector3 Location = Vector3(0.f);
	/// Rotation of the Transform.
	Vector3 Rotation = Vector3(0.f);
	/// Scale of the Transform.
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
