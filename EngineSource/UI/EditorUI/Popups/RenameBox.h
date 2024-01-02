#if EDITOR && 0
#pragma once
#include <UI/EditorUI/EditorPanel.h>

class RenameBox : public EditorPanel
{
public:
	RenameBox(std::string FileToRename, Vector2 Position);
	void UpdateLayout() override;
	~RenameBox();
	void OnButtonClicked(int Index) override;
	void Tick() override;
protected:
	UITextField* InputField;
	std::string File;
};

#endif