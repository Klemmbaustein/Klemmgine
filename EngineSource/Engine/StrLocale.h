#pragma once
#include <string>
#include <vector>

namespace Locale
{
	std::string GetLocalisedString(std::string Name);

	void SetLocale(std::string Name);
	std::string GetLocale();
}