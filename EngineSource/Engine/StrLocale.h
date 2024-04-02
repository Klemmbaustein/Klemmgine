#pragma once
#include <string>
#include <vector>

namespace Locale
{
	std::string GetLocalizedString(std::string Name);

	void SetLocale(std::string Name);
	std::string GetLocale();
}