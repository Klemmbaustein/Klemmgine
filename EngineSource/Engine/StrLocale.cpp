#include "StrLocale.h"
#include <Engine/File/Save.h>
#include <filesystem>
#include <Engine/Log.h>

namespace Locale
{
#if RELEASE
	static const std::string LocalePath = "Assets/Locale/";
#else
	static const std::string LocalePath = "Locale/";
#endif
	static std::string CurrentLocale = "EN";
	static SaveGame LocaleFile = SaveGame(LocalePath + CurrentLocale, "loc", false, false);
}

std::string Locale::GetLocalisedString(std::string Name)
{
	return LocaleFile.GetString(Name);
}

void Locale::SetLocale(std::string Name)
{
	CurrentLocale = Name;
	std::string Path = LocalePath + CurrentLocale;
	if (!std::filesystem::exists(Path + ".loc"))
	{
		Log::Print("Locale for language " + Name + " does not exist.");
		return;
	}
	Log::Print("Setting locale to \"" + Name + "\" (" + LocalePath + Name + ".loc)");
	LocaleFile = SaveGame(Path, "loc", false, false);
}

std::string Locale::GetLocale()
{
	return CurrentLocale;
}
