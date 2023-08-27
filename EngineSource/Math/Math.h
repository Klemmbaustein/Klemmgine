#pragma once

#include "Vector.h"
#include <string>
#include <cmath>

namespace Math
{
	inline constexpr const double PI = 3.14159265359;
	inline constexpr const float PI_F = (float)PI;
	bool IsPointIn2DBox(Vector2 BoxA, Vector2 BoxB, Vector2 Point);
	int SolveExpr(std::string expr);
	bool NearlyEqual(float A, float B, float epsilon = 0.005f);
}