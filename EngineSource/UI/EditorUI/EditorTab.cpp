#if EDITOR
#include "EditorTab.h"
#include <Math/Math.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>

void EditorTab::SetScale(Vector2 NewScale)
{
	Scale.X = std::max(NewScale.X, MinSize.X);
	Scale.Y = std::max(NewScale.Y, MinSize.Y);
	Scale.X = std::min(Scale.X, MaxSize.X);
	Scale.Y = std::min(Scale.Y, MaxSize.Y);
	TabBackground->SetPosition(Position);
	TabBackground->SetMinSize(Scale);
	UpdateLayout();
}

void EditorTab::SetPosition(Vector2 NewPosition)
{
	Position = NewPosition;
	TabBackground->SetPosition(Position);
	TabBackground->SetMinSize(Scale);
	UpdateLayout();
}

void EditorTab::UpdateTab()
{
	Vector2 ClampedMousePosition = Input::MouseLocation;
	if (Editor::DraggingTab && Editor::TabDragHorizontal)
	{
		ClampedMousePosition = Input::MouseLocation.Clamp(Vector2(Editor::DragMinMax.X, 0), Vector2(Editor::DragMinMax.Y, 0));
	}
	else if (Editor::DraggingTab)
	{
		ClampedMousePosition = Input::MouseLocation.Clamp(Vector2(0, Editor::DragMinMax.X), Vector2(0, Editor::DragMinMax.Y));
	}

	Vector2 BorderScale = Vector2(0.01) * Vector2(1, Graphics::AspectRatio);
	if (!Input::IsLMBDown)
	{
		if (IsDragged)
		{
			Editor::DraggingTab = false;
		}
		IsDragged = false;
		InitialMousePosition = 0;
	}
	if (IsDragged)
	{
		//Editor::CurrentUI->CurrentCursor = (IsDragHorizontal ? EditorUI::E_RESIZE_WE : EditorUI::E_RESIZE_NS);
	}

	if (Maths::NearlyEqual(ClampedMousePosition.Y, -1, BorderScale.Y)
		|| Maths::NearlyEqual(ClampedMousePosition.Y, 0.95, BorderScale.Y)
		|| Maths::NearlyEqual(ClampedMousePosition.Y, 1, BorderScale.Y))
	{
		return;
	}
	if (Maths::NearlyEqual(ClampedMousePosition.X, -1, BorderScale.X) || Maths::NearlyEqual(ClampedMousePosition.X, 1, BorderScale.X))
	{
		return;
	}

	bool HorizontalOverride = Editor::DraggingTab && !IsDragged && Editor::TabDragHorizontal
		&& (Maths::NearlyEqual(ClampedMousePosition.X, Position.X, 0.02)
		|| Maths::NearlyEqual(ClampedMousePosition.X, Position.X + Scale.X, 0.02));

	bool VerticalOverride = Editor::DraggingTab && !IsDragged && !Editor::TabDragHorizontal
		&& (Maths::NearlyEqual(ClampedMousePosition.Y, Position.Y, 0.02)
			|| Maths::NearlyEqual(ClampedMousePosition.Y, Position.Y + Scale.Y, 0.02));
	if (HorizontalOverride || VerticalOverride || Maths::IsPointIn2DBox(Position, Position + Scale, ClampedMousePosition) && !IsDragged)
	{
		if (HorizontalOverride || (!VerticalOverride && !Maths::IsPointIn2DBox(Position + BorderScale * Vector2(1, 0),
			Position + Scale - BorderScale * Vector2(1, 0), ClampedMousePosition)))
		{
			if (!HorizontalOverride && IsMouseDown)
			{
				return;
			}
			if (Input::IsLMBDown)
			{
				IsDragged = true;
				Editor::DraggingTab = true;
				Editor::TabDragHorizontal = true;
				IsDragHorizontal = true;
			}
			Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_WE;
		}
		if (VerticalOverride || (!HorizontalOverride && !Maths::IsPointIn2DBox(Position + BorderScale * Vector2(0, 1),
			Position + Scale - BorderScale * Vector2(0, 1), ClampedMousePosition)))
		{
			if (!VerticalOverride && IsMouseDown)
			{
				return;
			}
			if (Input::IsLMBDown)
			{
				IsDragHorizontal = false;
				Editor::DraggingTab = true;
				Editor::TabDragHorizontal = false;
				IsDragged = true;

			}
			Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_NS;

		}
	}
	if (!IsMouseDown && Input::IsLMBDown)
	{
		InitialMousePosition = ClampedMousePosition;
		InitialScale = Scale;
		InitialPosition = Position;
	}
	IsMouseDown = Input::IsLMBDown;

	if (!IsDragged)
	{

		IsMouseDown = Input::IsLMBDown;
		return;
	}
	bool IsDraggingScale;
	if (IsDragHorizontal)
	{
		if (InitialMousePosition.X > InitialPosition.X + InitialScale.X / 2)
		{
			IsDraggingScale = true;
			Editor::NewDragMinMax.X = std::max(Position.X + MinSize.X, Editor::NewDragMinMax.X);
			Editor::NewDragMinMax.Y = std::min(Position.X + MaxSize.X, Editor::NewDragMinMax.Y);
		}
		else
		{
			IsDraggingScale = false;
			Editor::NewDragMinMax.X = std::max(Position.X + InitialScale.X - MaxSize.X, Editor::NewDragMinMax.X);
			Editor::NewDragMinMax.Y = std::min(Position.X + InitialScale.X - MinSize.X, Editor::NewDragMinMax.Y);
		}
	}
	else
	{
		if (InitialMousePosition.Y > InitialPosition.Y + InitialScale.Y / 2)
		{
			IsDraggingScale = true;

			Editor::NewDragMinMax.X = std::max(InitialPosition.Y + MinSize.Y, Editor::NewDragMinMax.X);
			Editor::NewDragMinMax.Y = std::min(InitialPosition.Y + MaxSize.Y, Editor::NewDragMinMax.Y);
		}
		else
		{
			IsDraggingScale = false;
			Editor::NewDragMinMax.X = std::max(InitialPosition.Y + InitialScale.Y - MaxSize.Y, Editor::NewDragMinMax.X);
			Editor::NewDragMinMax.Y = std::min(InitialPosition.Y + InitialScale.Y - MinSize.Y, Editor::NewDragMinMax.Y);
		}
	}

	if (IsDraggingScale)
	{
		if (IsDragHorizontal)
		{
			SetScale(Vector2(ClampedMousePosition.X - Position.X, Scale.Y));
		}
		else
		{
			SetScale(Vector2(Scale.X, ClampedMousePosition.Y - Position.Y));
		}
		return;
	}

	if (IsDragHorizontal)
	{
		float PositionDifference = Position.X - ClampedMousePosition.X;

		if (Scale.X + PositionDifference < MinSize.X)
		{
			PositionDifference = MinSize.X - Scale.X;
		}

		Position.X -= PositionDifference;
		SetScale(Vector2(Scale.X + PositionDifference, Scale.Y));
	}
	else
	{
		float PositionDifference = Position.Y - ClampedMousePosition.Y;

		if (Scale.Y + PositionDifference < MinSize.Y)
		{
			PositionDifference = MinSize.Y - Scale.Y;
		}

		Position.Y -= PositionDifference;
		SetScale(Vector2(Scale.X, Scale.Y + PositionDifference));
	}

}
#endif