#pragma once
#define _USE_MATH_DEFINES
#include "Vector.h"
#include <string>
#include <math.h>
namespace Maths
{
	inline constexpr const double PI = 3.14159265359;
	bool IsPointIn2DBox(Vector2 BoxA, Vector2 BoxB, Vector2 Point);
	int SolveExpr(std::string expr);
	bool NearlyEqual(float A, float B, float epsilon = 0.005f);
}