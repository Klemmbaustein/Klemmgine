#if EDITOR
#pragma once
#include <UI/EditorUI/Popups/EditorPopup.h>
#include <UI/UIfwd.h>

class BakeMenu : public EditorPopup
{
public:
	BakeMenu();
	~BakeMenu();
	void OnButtonClicked(int Index) override;
	void Tick() override;
	UITextField* InputFields[2] = { nullptr, nullptr };
	size_t CurrentBakeProgress = 0;
	UIText* BakeProgressText = nullptr;
	static bool BakeMenuActive;

	void GenerateBakeLog();

	void StartBake();
	bool IsFinished = false;
	UIScrollBox* LogScrollBox = nullptr;
protected:
};
#endif