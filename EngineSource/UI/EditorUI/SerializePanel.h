#if EDITOR
#pragma once
#include "EditorPanel.h"
#include <Engine/File/SaveData.h>

namespace Editor::SerializePanel
{
	std::string GetLayoutPrefFilePath();

	/**
	 * @brief
	 * Serializes a panel to a SaveData representation, that can be saved to a file.
	 * 
	 * @param Target
	 * The panel to serialize.
	 */
	SaveData::Field SerializeLayout(EditorPanel* Target);

	/**
	 * @brief
	 * DeSerializes a panel from a SaveData representation.
	 * 
	 * @param From
	 * The SaveData field which contains the panel data. Usually /EditorPath/EditorContent/Config/EditorLayout.pref -> root
	 * 
	 * @param Parent
	 * The parent of the new panel structure.
	 * 
	 * @return
	 * The root panel of the constructed panel structure.
	 */
	EditorPanel* DeSerializeLayout(SaveData::Field From, EditorPanel* Parent = nullptr);
}
#endif