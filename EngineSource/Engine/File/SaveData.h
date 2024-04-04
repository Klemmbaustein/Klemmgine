#pragma once
#include <string>
#include <Engine/TypeEnun.h>
#include <Math/Vector.h>
#include <set>

struct SaveData
{
	struct Field
	{
		Field();
		Field(NativeType::NativeType Type, std::string Name, std::string Data);

		std::string Name = "";
		std::string Data;
		NativeType::NativeType Type = NativeType::Null;

		std::vector<Field> Children;

		bool operator<(Field b) const;

		std::string Serialize(size_t Depth) const;

		Field& At(std::string Name);

		int AsInt() const;
		bool AsBool() const;
		std::string AsString() const;
		float AsFloat() const;
		Vector3 AsVector() const;
	};

	SaveData();
	SaveData(std::string SaveName, std::string Extension = "kesv", bool InSaveFolder = true, bool ShouldSaveOnClose = true);
	static SaveData ParseString(std::string Str);

	std::string SerializeString() const;

	Field& GetField(std::string Name);

	Field GetField(std::string Name) const;

	bool HasField(std::string Name);

	void SetField(Field S);

	Field GetPropertyOfType(std::string Name, NativeType::NativeType PropertyType) const;

	int GetInt(std::string Name) const;
	bool GetBool(std::string Name) const;
	std::string GetString(std::string Name) const;
	float GetFloat(std::string Name) const;
	Vector3 GetVector(std::string Name) const;

	void ClearFields();
	~SaveData();
	
	void SaveToFile(std::string File) const;

	bool SaveGameIsNew() const;

	std::vector<Field>& GetAllFields();

private:
	static void Error(std::string Message);
	std::vector<Field> Fields;
	std::string OpenedSave;
	bool IsNew = true;
	bool ShouldSave = true;
};