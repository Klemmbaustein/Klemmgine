#if EDITOR
#pragma once
#include <UI/EditorUI/Popups/EditorPopup.h>
#include <UI/UIfwd.h>

class RenameBox : public EditorPopup
{
public:
	RenameBox(std::string FileToRename);
	~RenameBox();
	void OnButtonClicked(int Index) override;
	void Tick() override;
protected:
	UITextField* InputField;
	std::string File;
};

#endif