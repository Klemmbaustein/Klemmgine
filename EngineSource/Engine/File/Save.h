#pragma once
#include <string>
#include <map>
#include <Engine/TypeEnun.h>
#include <Math/Vector.h>


struct SaveGame
{
	SaveGame(std::string SaveName, std::string Extension = "save", bool InSaveFolder = true, bool ShouldSaveOnClose = true);
	struct SaveProperty
	{
		SaveProperty(std::string Name, std::string Value, Type::TypeEnum Type);
		SaveProperty() {}
		std::string Name = "";
		std::string Value = "";
		Type::TypeEnum Type = Type::Null;
		auto operator<=>(SaveProperty const&) const = default;
	};

	SaveProperty GetProperty(std::string Name)  const;

	void SetProperty(SaveProperty S);

	SaveProperty GetPropertyOfType(std::string Name, Type::TypeEnum PropertyType) const;

	int GetInt(std::string Name) const;
	bool GetBool(std::string Name) const;
	std::string GetString(std::string Name) const;
	float GetFloat(std::string Name) const;
	Vector3 GetVector(std::string Name) const;

	void ClearProperties()
	{
		Properties.clear();
	}
	~SaveGame();

	bool SaveGameIsNew() const;

	std::map<std::string, SaveProperty> GetProperties() const;

private:
	std::map<std::string, SaveProperty> Properties;
	std::string OpenedSave;
	bool IsNew = true;
	bool ShouldSave = true;
};