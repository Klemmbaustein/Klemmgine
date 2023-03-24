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
		Editor::CurrentUI->CurrentCursor = (IsDragHorizontal ? EditorUI::E_RESIZE_WE : EditorUI::E_RESIZE_NS);
	}

	bool HorizontalOverride = Editor::DraggingTab && !IsDragged && Editor::TabDragHorizontal
		&& (Maths::NearlyEqual(Input::MouseLocation.X, Position.X, 0.06)
		|| Maths::NearlyEqual(Input::MouseLocation.X, Position.X + Scale.X, 0.06));
	bool VerticalOverride = Editor::DraggingTab && !IsDragged && !Editor::TabDragHorizontal
		&& (Maths::NearlyEqual(Input::MouseLocation.Y, Position.Y, 0.06)
			|| Maths::NearlyEqual(Input::MouseLocation.Y, Position.Y + Scale.Y, 0.06));

	if (HorizontalOverride || VerticalOverride || Maths::IsPointIn2DBox(Position, Position + Scale, Input::MouseLocation) && !IsDragged)
	{
		if (HorizontalOverride || !Maths::IsPointIn2DBox(Position + BorderScale * Vector2(1, 0),
			Position + Scale - BorderScale * Vector2(1, 0), Input::MouseLocation))
		{
			if (Maths::NearlyEqual(Input::MouseLocation.X, -1, BorderScale.X) || Maths::NearlyEqual(Input::MouseLocation.X, 1, BorderScale.X))
			{
				return;
			}
			if (!HorizontalOverride && IsMouseDown)
			{
				return;
			}
			if (Input::IsLMBDown)
			{
				IsMouseDown = true,
				IsDraggingScale = !Maths::NearlyEqual(Input::MouseLocation.X, Position.X, 0.1);
				IsDragged = true;
				Editor::DraggingTab = true;
				Editor::TabDragHorizontal = true;
				IsDragHorizontal = true;
				if (InitialMousePosition == 0)
				{
					InitialMousePosition = Input::MouseLocation;
					InitialScale = Scale;
					InitialPosition = Position;
				}
			}
			Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_WE;
		}
		if (VerticalOverride || (!Maths::IsPointIn2DBox(Position + BorderScale * Vector2(0, 1), Position + Scale - BorderScale * Vector2(0, 1), Input::MouseLocation)))
		{
			if (Maths::NearlyEqual(Input::MouseLocation.Y, -1, BorderScale.Y) || Maths::NearlyEqual(Input::MouseLocation.Y, 1, BorderScale.Y))
			{
				return;
			}
			if (!VerticalOverride && IsMouseDown)
			{
				return;
			}
			if (Input::IsLMBDown)
			{
				IsMouseDown = true;
				IsDraggingScale = !Maths::NearlyEqual(Input::MouseLocation.Y, Position.Y, BorderScale.Y * 5);
				IsDragHorizontal = false;
				Editor::TabDragHorizontal = false;
				IsDragged = true;
				if (InitialMousePosition == 0)
				{
					InitialMousePosition = Input::MouseLocation;
					InitialScale = Scale;
					InitialPosition = Position;
				}
			}
			Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_NS;

		}
	}
	if (!IsDragged)
	{
		IsMouseDown = Input::IsLMBDown;
		return;
	}
	if (IsDragHorizontal)
	{
		if (InitialMousePosition.X > InitialPosition.X + InitialScale.X / 2)
		{
			Editor::DragMinMax.X = std::max(Position.X + MinSize.X, Editor::DragMinMax.X);
			Editor::DragMinMax.Y = std::min(Position.X + MaxSize.X, Editor::DragMinMax.Y);
		}
		else
		{
			Editor::DragMinMax.X = std::max(Position.X + InitialScale.X - MaxSize.X, Editor::DragMinMax.X);
			Editor::DragMinMax.Y = std::min(Position.X + InitialScale.X - MinSize.X, Editor::DragMinMax.Y);
		}
	}
	else
	{
		if (InitialMousePosition.Y > InitialPosition.Y + InitialScale.Y / 2)
		{
			Editor::DragMinMax.X = std::max(InitialPosition.Y + MinSize.Y, Editor::DragMinMax.X);
			Editor::DragMinMax.Y = std::min(InitialPosition.Y + MaxSize.Y, Editor::DragMinMax.Y);
		}
		else
		{
			Editor::DragMinMax.X = std::max(InitialPosition.Y + InitialScale.Y - MaxSize.Y, Editor::DragMinMax.X);
			Editor::DragMinMax.Y = std::min(InitialPosition.Y + InitialScale.Y - MinSize.Y, Editor::DragMinMax.Y);
		}
	}
	Vector2 ClampedMousePosition;
	if (IsDragHorizontal)
	{
		ClampedMousePosition = Input::MouseLocation.Clamp(Vector2(Editor::DragMinMax.X, -1), Vector2(Editor::DragMinMax.Y, 1));
	}
	else
	{
		ClampedMousePosition = Input::MouseLocation.Clamp(Vector2(-1, Editor::DragMinMax.X), Vector2(1, Editor::DragMinMax.Y));
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
