#if EDITOR
#pragma once
#include <UI/Default/UICanvas.h>
#include <UI/UIBackground.h>

class EditorPopup : public UICanvas
{
public:
	struct PopupOption
	{
		std::string Name;
		void(*OnClicked)() = nullptr;
		bool Close = true;
	};

private:
	std::vector<PopupOption> Options;
protected:
	void SetOptions(std::vector<PopupOption> NewOptions);
	UIBackground* TitleBackground = nullptr;
	UIBox* RootBox = nullptr;
	UIBackground* PopupBackground = nullptr;
	UIBackground* OptionsList = nullptr;
public:
	EditorPopup(Vector2 Position, Vector2 Scale, std::string Name);
	virtual ~EditorPopup();

	void HandlePopupButtons(int Index);
	void TickPopup();
};
#endif