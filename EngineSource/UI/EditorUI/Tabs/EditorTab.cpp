#if EDITOR
#include "EditorTab.h"
#include <UI/EditorUI/Viewport.h>

void EditorTab::UpdateLayout()
{
}

EditorTab::EditorTab(Vector3* UIColors)
{
	this->UIColors = UIColors;
	TabBackground = new UIBackground(false, Viewport::ViewportInstance->Position, UIColors[0], Viewport::ViewportInstance->Scale);
	TabBackground->SetMaxSize(Viewport::ViewportInstance->Scale);
	TabBackground->SetBorder(UIBox::E_DARKENED_EDGE, 0.2f);
	TabBackground->IsVisible = false;
}
#endif