#include "EngineRandom.h"
#include <random>

int Random::GetRandomNumber(int Min, int Max)
{
	int Range = std::abs(Max - Min);
	int RandomNumber = std::rand() % Range;
	return RandomNumber + Min;
}

float Random::GetRandomNumber(float Min, float Max)
{
	float Range = std::abs(Max - Min);
	float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	return r * Range + Min;
}
