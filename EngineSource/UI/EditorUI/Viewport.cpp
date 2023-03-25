#include "Viewport.h"
#if EDITOR

Viewport::Viewport(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorTab(Colors, Position, Scale, 0, 2)
{
	TabBackground->SetOpacity(0.8);
}

void Viewport::Save()
{
}

void Viewport::Load(std::string File)
{
}
void Viewport::UpdateLayout()
{
}
void Viewport::Tick()
{
	UpdateTab();
}
#endif
