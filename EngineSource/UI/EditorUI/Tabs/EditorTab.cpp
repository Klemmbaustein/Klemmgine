#if EDITOR
#include "EditorTab.h"
#include <UI/EditorUI/Viewport.h>
#include <Engine/Utility/FileUtility.h>

void EditorTab::Save()
{
}

void EditorTab::Load(std::string File)
{
}

EditorTab::EditorTab(EditorPanel* Parent, std::string Name, std::string File)
	: EditorPanel(Parent, Name + ": " + FileUtil::GetFileNameWithoutExtensionFromPath(File))
{
	CanBeClosed = true;
}
EditorTab::EditorTab(Vector2 Position, Vector2 Scale, std::string Name, std::string File)
	: EditorPanel(Position, Scale, Name + ": " + FileUtil::GetFileNameWithoutExtensionFromPath(File))
{
	CanBeClosed = true;
}
#endif