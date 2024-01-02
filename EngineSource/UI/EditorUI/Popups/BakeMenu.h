#if EDITOR && 0
#pragma once
#include <UI/EditorUI/EditorPanel.h>
#include <UI/UIfwd.h>

class BakeMenu : public EditorPanel
{
public:
	BakeMenu();
	void UpdateLayout() override;
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