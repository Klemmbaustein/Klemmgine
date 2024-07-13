#pragma once
#include <string>
#include <Engine/TypeEnun.h>
#include <Math/Vector.h>
#include <set>

/**
* @brief
* SaveData format.
*/
struct SaveData
{
	/**
	* @brief
	* A SaveData field.
	*/
	struct Field
	{
		Field();
		Field(NativeType::NativeType Type, std::string Name, std::string Data);

		/// The name of this field.
		std::string Name = "";
		/// The value of the data in this field.
		std::string Data;
		/// The type of the field.
		NativeType::NativeType Type = NativeType::Null;

		/**
		* @brief
		* Children of this field, if this field is an object.
		*/
		std::vector<Field> Children;

		bool operator<(Field b) const;

		/**
		* @brief
		* Converts this field to a string.
		* 
		* @param Depth
		* This amount of indentation (tabs) will be added in front of the string representation.
		*/
		std::string Serialize(size_t Depth) const;

		/// Gets the child of this field with the given name.
		Field& At(std::string Name);
		/// Checks if this field has a child with the given name.
		bool Contains(std::string Name);

		/// Gets the value of this field as an int.
		int AsInt() const;
		/// Gets the value of this field as a bool.
		bool AsBool() const;
		/// Gets the value of this field as a string.
		std::string AsString() const;
		/// Gets the value of this field as a float.
		float AsFloat() const;
		/// Gets the value of this field as a vector.
		Vector3 AsVector() const;
	};

	SaveData();

	/**
	* @brief
	* Loads a save file.
	* 
	* @param SaveName
	* The name of the save file.
	* 
	* @param Extension
	* The extension used for the save file.
	* 
	* @param InSaveFolder
	* True if the save file should be in the Saves/ directory.
	* 
	* @param SaveOnDestruct
	* Should the save file be updated once this object is destructed.
	*/
	SaveData(std::string SaveName, std::string Extension = "kesv", bool InSaveFolder = true, bool SaveOnDestruct = true);
	SaveData(const SaveData&) = delete;

	/// Parses a string to save data.
	static SaveData ParseString(std::string Str);

	/// Converts the save data to a string.
	std::string SerializeString() const;

	/// Gets a reference to a field with the given name.
	Field& GetField(std::string Name);

	/// Gets a field with the given name.
	Field GetField(std::string Name) const;

	/// Returns true if this field exists in the save data.
	bool HasField(std::string Name);

	/// Adds the field to the save data.
	void SetField(Field S);

	Field GetPropertyOfType(std::string Name, NativeType::NativeType PropertyType) const;

	/// Gets the value of the field with the given name as an int.
	int GetInt(std::string Name) const;
	/// Gets the value of the field with the given name as a bool.
	bool GetBool(std::string Name) const;
	/// Gets the value of the field with the given name as a string.
	std::string GetString(std::string Name) const;
	/// Gets the value of the field with the given name as a float.
	float GetFloat(std::string Name) const;
	/// Gets the value of the field with the given name as a vector.
	Vector3 GetVector(std::string Name) const;

	/// Clears all fields.
	void ClearFields();
	~SaveData();
	
	/**
	* @brief
	* Saves this save data to a file.
	*/
	void SaveToFile(std::string File) const;

	/// True if this save data file is new, false if it already existed before.
	bool SaveGameIsNew() const;

	/// Gets all fields of this file.
	std::vector<Field>& GetAllFields();

private:
	static void Error(std::string Message);
	std::vector<Field> Fields;
	std::string OpenedSave;
	bool IsNew = true;
	bool ShouldSave = true;
};