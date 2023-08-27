#pragma once
#include <string>
#include <map>
#include <Engine/TypeEnun.h>

struct SaveGame
{
	SaveGame(std::string SaveName, std::string Extension = "save", bool InSaveFolder = true);
	struct SaveProperty
	{
		SaveProperty(std::string Name, std::string Value, Type::TypeEnum Type);
		SaveProperty() {}
		std::string Name = ""; std::string Value = ""; Type::TypeEnum Type = Type::Null;
		auto operator<=>(SaveProperty const&) const = default;
	};

	SaveProperty GetPropterty(std::string Name);

	void SetPropterty(SaveProperty S);
	void ClearProperties()
	{
		Properties.clear();
	}
	~SaveGame();

	bool SaveGameIsNew();

	std::map<std::string, SaveProperty> GetProperties();

private:
	std::map<std::string, SaveProperty> Properties;
	std::string OpenedSave;
	bool IsNew = true;
};