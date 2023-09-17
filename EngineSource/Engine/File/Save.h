#pragma once
#include <string>
#include <map>
#include <Engine/TypeEnun.h>
#include <Math/Vector.h>

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

	SaveProperty GetProperty(std::string Name);

	void SetProperty(SaveProperty S);

	SaveProperty GetPropertyOfType(std::string Name, Type::TypeEnum PropertyType);

	int GetInt(std::string Name);
	bool GetBool(std::string Name);
	std::string GetString(std::string Name);
	float GetFloat(std::string Name);
	Vector3 GetVector(std::string Name);

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