#include "EngineRandom.h"
#include <random>
#include <iostream>

int Random::GetRandomInt(int Min, int Max)
{
	int Range = std::abs(Max - Min);
	if (Range == 0)
	{
		return Min;
	}
	int RandomNumber = std::rand() % Range;
	return RandomNumber + Min;
}

float Random::GetRandomFloat(float Min, float Max)
{
	float Range = std::abs(Max - Min);
	float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return r * Range + Min;
}
