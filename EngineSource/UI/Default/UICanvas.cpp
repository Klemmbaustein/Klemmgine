#include "UICanvas.h"

void UICanvas::Tick()
{
}

void UICanvas::OnButtonClicked(int Index)
{

}

void UICanvas::OnButtonDragged(int Index)
{
}


UICanvas::~UICanvas()
{
	for (size_t i = 0; i < Graphics::UIToRender.size(); i++)
	{
		if (Graphics::UIToRender[i] == this)
		{
			Graphics::UIToRender.erase(Graphics::UIToRender.begin() + i);
			return;
		}
	}
}
