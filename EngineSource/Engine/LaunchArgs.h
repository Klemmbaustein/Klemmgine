#pragma once
#include <vector>
#include <string>

namespace LaunchArgs
{
	void Evaluate(int argc, char** argv);
	void EvaluateVector(std::vector<std::string> Args);
}