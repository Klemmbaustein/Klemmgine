#include "Vector.h"
#include <iostream>
#include <numeric>
#include <iomanip>
#include <sstream>
#include <Math/Math.h>
#include <glm/gtx/euler_angles.hpp>
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <Engine/EngineError.h>

Vector3 operator+(Vector3 a, Vector3 b)
{
	return Vector3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
}

Vector3 operator-(Vector3 a, Vector3 b)
{
	return Vector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
}

Vector3 operator*(Vector3 a, Vector3 b)
{
	return Vector3(a.X * b.X, a.Y * b.Y, a.Z * b.Z);
}

Vector3 operator*(Vector3 a, float b)
{
	return Vector3(a.X * b, a.Y * b, a.Z * b);
}

Vector3 operator/(Vector3 a, Vector3 b)
{
	return Vector3(a.X / b.X, a.Y / b.Y, a.Z / b.Z);
}

Vector2 operator+(Vector2 a, Vector2 b)
{
	return Vector2(a.X + b.X, a.Y + b.Y);
}

Vector2 operator-(Vector2 a, Vector2 b)
{
	return Vector2(a.X - b.X, a.Y - b.Y);
}

Vector2 operator*(Vector2 a, float b)
{
	return Vector2(a.X * b, a.Y * b);
}

Vector2 operator/(Vector2 a, Vector2 b)
{
	return Vector2(a.X / b.X, a.Y / b.Y);
}

Vector2 operator/(Vector2 a, float b)
{
	return Vector2(a.X / b, a.Y / b);
}

bool operator==(Vector2 a, float b)
{
	return a.X == b && a.Y == b;
}

inline float _nearlyequal(float a, float b, float t)
{
	return a > b - t && a < b + t;
}

bool Vector3::NearlyEqual(Vector3 a, Vector3 b, float Threshold)
{
	return _nearlyequal(a.X, b.X, Threshold) && _nearlyequal(a.Y, b.Y, Threshold) && _nearlyequal(a.Z, b.Z, Threshold);
}

Vector3 Vector3::Clamp(Vector3 v, Vector3 min, Vector3 max)
{
	Vector3 ret;
	ret.X = std::max(v.X, min.X);
	ret.Y = std::max(v.Y, min.Y);
	ret.Z = std::max(v.Z, min.Z);
	ret.X = std::min(ret.X, max.X);
	ret.Y = std::min(ret.Y, max.Y);
	ret.Z = std::min(ret.Z, max.Z);
	return ret;
}

bool operator==(Vector2 a, Vector2 b)
{
	return (a.X == b.X && a.Y == b.Y);
}

bool operator==(Vector3 a, Vector3 b)
{
	return a.X == b.X && a.Y == b.Y && b.Z == a.Z;
}

bool operator==(Transform a, Transform b)
{
	return a.Position == b.Position && a.Rotation == b.Rotation && a.Scale == b.Scale;
}

Transform operator+(Transform a, Transform b)
{
	return Transform(a.Position + b.Position, a.Rotation + b.Rotation, a.Scale * b.Scale);
}


Vector3 Vector3::Normalize() const
{
	float Len = sqrt(X * X + Y * Y + Z * Z);
	if (Len > 0)
	{
		return Vector3(X, Y, Z) / Len;
	}
	return Vector3(0);
}

float Vector2::Length() const
{
	return sqrt(X * X + Y * Y);
}

Vector2 Vector2::Normalize() const
{
	float Len = Length();
	if (Len > 0)
	{
		return Vector2(X, Y) / Len;
	}
	return Vector2(0);
}

Vector2 Vector2::Clamp(Vector2 Min, Vector2 Max) const
{
	float NewX = std::min(Max.X, std::max(Min.X, X));
	float NewY = std::min(Max.Y, std::max(Min.Y, Y));
	return Vector2(NewX, NewY);
}

Vector2 Vector2::Min(Vector2 A, Vector2 B)
{
	float NewX = std::min(A.X, B.X);
	float NewY = std::min(A.Y, B.Y);
	return Vector2(NewX, NewY);
}

Vector2 Vector2::Max(Vector2 A, Vector2 B)
{
	float NewX = std::max(A.X, B.X);
	float NewY = std::max(A.Y, B.Y);
	return Vector2(NewX, NewY);
}

float Vector3::Length() const
{
	return sqrt(X * X + Y * Y + Z * Z);
}

Vector3& Vector3::operator+=(Vector3 a)
{
	X = a.X + X;
	Y = a.Y + Y;
	Z = a.Z + Z;
	return *this;
}

Vector3& Vector3::operator-=(Vector3 a)
{
	X = X - a.X;
	Y = Y - a.Y;
	Z = Z - a.Z;
	return *this;
}

Vector3 Vector3::RadiansToDegrees() const
{
	return Vector3(glm::degrees(X), glm::degrees(Y), glm::degrees(Z));
}
Vector3 Vector3::operator-()
{
	return Vector3() - *this;
}

Vector3 Vector3::DegreesToRadians() const
{
	return Vector3(glm::radians(X), glm::radians(Y), glm::radians(Z));
}

std::string Vector3::ToString() const
{
	return std::string(std::to_string(X) + " " + std::to_string(Y) + " " + std::to_string(Z));
}

std::string Vector2::ToString() const
{
	return std::string(std::to_string(X) + " " + std::to_string(Y));
}

Vector3::Vector3(glm::vec3 xyz)
{
	X = xyz.x;
	Y = xyz.y;
	Z = xyz.z;
}

float& Vector3::at(unsigned int Index)
{
	return *((float*)this + Index);
}

Vector3 Vector3::Abs()
{
	return Vector3(abs(X), abs(Y), abs(Z));
}

Vector3 Vector3::ProjectToPlane(Vector3 PlaneOrigin, Vector3 PlaneNormal)
{
	Vector3 normalizedPlaneNormal = PlaneNormal.Normalize();
	Vector3 pointProjection = *this - Vector3::Dot(*this - PlaneOrigin, normalizedPlaneNormal) * normalizedPlaneNormal;
	return pointProjection;
}

Vector3 Vector3::GetForwardVector(Vector3 In)
{
	return Vector3::GetScaledAxis(In.DegreesToRadians(), 2);
}

Vector3 Vector3::GetRightVector(Vector3 In)
{
	return GetScaledAxis(In.DegreesToRadians(), 0);
}

Vector3 Vector3::SnapToGrid(Vector3 In, Vector3 GridSize)
{
	Vector3 Result = Vector3((float)(int)(In.X / GridSize.X), (float)(int)(In.Y / GridSize.Y), (float)(int)(In.Z / GridSize.Z));
	Result = Result * GridSize;
	return Result;
}

Vector3 Vector3::Cross(Vector3 a, Vector3 b)
{
	return Vector3(a.Y * b.Z - a.Z * b.Y,
		a.Z * b.X - a.X * b.Z,
		a.X * b.Y - a.Y * b.X);
}

float Vector3::Dot(Vector3 a, Vector3 b)
{
	float result = 0.0;
	for (int i = 0; i < 3; i++)
		result += a[i] * b[i];
	return result;
}

Vector3 Vector3::LookAtFunctionY(Vector3 Start, Vector3 End, bool Radiants)
{
	Vector3 Dir = (End - Start).Normalize();
	glm::quat quat = glm::quatLookAt((glm::vec3)Dir, glm::vec3(0, 0, 1));
	glm::vec3 Euler = glm::eulerAngles(quat) + glm::vec3(-Math::PI_F / 2, 0, 0);
	if (Radiants)
		return Euler;
	else
		return Vector3(Euler).RadiansToDegrees();
}

Vector3 Vector3::LookAtFunction(Vector3 Start, Vector3 End, bool Radiants)
{
	Vector3 Dir = (End - Start).Normalize();
	glm::quat quat = glm::quatLookAt((glm::vec3)Dir, glm::vec3(0, 1, 0));
	glm::vec3 Euler = glm::eulerAngles(quat);

	if (Radiants)
		return Vector3(Euler);
	else
		return Vector3(Euler).RadiansToDegrees();
}

Vector3 Vector3::QuatToEuler(glm::quat quat)
{
	glm::vec3 angles = glm::eulerAngles(quat);
	return Vector3(angles);
}

float Vector3::Distance(Vector3 a, Vector3 b)
{
	Vector3 dif = b - a;
	return dif.Length();
}

float Vector3::DistanceSquared(Vector3 a, Vector3 b)
{
	Vector3 dif = b - a;
	return dif.X * dif.X + dif.Y * dif.Y + dif.Z * dif.Z;
}

Vector3 Vector3::GetUpVector(Vector3 In)
{
	return Vector3::GetScaledAxis(In.DegreesToRadians(), 1);
}
typedef float Float;
typedef Float Axis[3];
typedef Axis Axes[3];
static void copy(const Axes& from, Axes& to)
{
	for (size_t i = 0; i != 3; ++i) 
	{
		for (size_t j = 0; j != 3; ++j)
		{
			to[i][j] = from[i][j];
		}
	}
}
static void mul(Axes& mat, Axes& b)
{
	Axes result = {};
	for (size_t i = 0; i != 3; ++i)
	{
		for (size_t j = 0; j != 3; ++j)
		{
			Float sum = 0;
			for (size_t k = 0; k != 3; ++k)
			{
				sum += mat[i][k] * b[k][j];
			}
			result[i][j] = sum;
		}
	}
	copy(result, mat);
}

Vector3 Vector3::GetScaledAxis(Vector3 Rot, unsigned int Dir)
{
	float x = -Rot.X;
	float y = Rot.Y;
	float z = -Rot.Z;
	Axes matX = {
	  {1,     0,     0 },
	  {0, cosf(x),sinf(x)},
	  {0,-sinf(x),cosf(x)}
	};
	Axes matY = {
	  {cosf(y),0,-sinf(y)},
	  {     0,1,      0},
	  {sinf(y),0, cosf(y)}
	};
	Axes matZ = {
	  { cosf(z),sinf(z),0},
	  {-sinf(z),cosf(z),0},
	  {      0,     0,1}
	};
	Axes axes = {
	  {1,0,0},
	  {0,1,0},
	  {0,0,1}
	};
	mul(axes, matX);
	mul(axes, matY);
	mul(axes, matZ);

	return Vector3(axes[Dir][2], axes[Dir][1], axes[Dir][0]).Normalize();
}

Vector3 Vector3::RotateVector(Vector3 Vec, Vector3 Rot)
{
	return glm::vec3(Transform(0, Rot, 1).ToMatrix() * glm::vec4((glm::vec3)Vec, 1));
}

Vector3 Vector3::TranslateVector(Vector3 Vec, Transform Transform, bool Degrees)
{
	if (Degrees)
	{
		Transform.Rotation = Transform.Rotation.DegreesToRadians();
	}
	return glm::vec3(Transform.ToMatrix() * glm::vec4((glm::vec3)Vec, 1));
}

Vector3 Vector3::FromString(std::string In)
{
	if (In.size() == 0)
	{
		return Vector3();
	}
	std::string myString = In;
	std::stringstream iss(myString);

	float number;
	std::vector<float> myNumbers;
	while (iss >> number)
		myNumbers.push_back(number);
	if(myNumbers.size() >= 3)
		return Vector3(myNumbers.at(0), myNumbers.at(1), myNumbers.at(2));
	return Vector3();
}
Vector3 Vector3::Lerp(Vector3 a, Vector3 b, float val)
{
	return Vector3(std::lerp(a.X, b.X, val), std::lerp(a.Y, b.Y, val), std::lerp(a.Z, b.Z, val));
}

Vector2& Vector2::operator+=(Vector2 b)
{
	X = X + b.X;
	Y = Y + b.Y;
	return *this;
}

Vector2& Vector2::operator*=(Vector2 b)
{
	X = X * b.X;
	Y = Y * b.Y;
	return *this;
}

Vector2 Vector2::operator*(Vector2 b)
{
	return Vector2(X * b.X, Y * b.Y);
}


glm::mat4 Transform::ToMatrix() const
{
	glm::mat4 Matrix = glm::mat4(1);

	Matrix = glm::translate(Matrix, (glm::vec3)Position);
	Matrix = glm::rotate(Matrix, Rotation.Z, glm::vec3(0, 0, 1));
	Matrix = glm::rotate(Matrix, Rotation.Y, glm::vec3(0, 1, 0));
	Matrix = glm::rotate(Matrix, Rotation.X, glm::vec3(1, 0, 0));
	Matrix = glm::scale(Matrix, (glm::vec3)Scale);

	return Matrix;
}
