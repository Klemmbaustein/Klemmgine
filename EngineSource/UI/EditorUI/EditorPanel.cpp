#if EDITOR
#include "EditorPanel.h"
#include <Math/Math.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>

void EditorPanel::SetScale(Vector2 NewScale)
{
	Vector2 clampedScale;
	clampedScale.X = std::max(NewScale.X, MinSize.X);
	clampedScale.Y = std::max(NewScale.Y, MinSize.Y);
	clampedScale.X = std::min(clampedScale.X, MaxSize.X);
	clampedScale.Y = std::min(clampedScale.Y, MaxSize.Y);
	if (Scale != clampedScale)
	{
		if (!IsPopup)
		{
			TabBackground->SetPosition(Position);
			TabBackground->SetMinSize(clampedScale);
		}
		else
		{
			TabBackground->SetMinSize(clampedScale);
			MainBackground->SetPosition(Position);
			MainBackground->SetMinSize(clampedScale);
			TitleBackground->SetMinSize(Vector2(clampedScale.X, 0));
		}
		Scale = clampedScale;
		UpdateLayout();
	}
}

void EditorPanel::SetPosition(Vector2 NewPosition)
{
	Position = NewPosition;
	if (!IsPopup)
	{
		TabBackground->SetPosition(Position);
		TabBackground->SetMinSize(Scale);
	}
	else
	{
		TabBackground->SetMinSize(Scale);
		MainBackground->SetPosition(Position);
		MainBackground->SetMinSize(Scale);
		TitleBackground->SetMinSize(Vector2(Scale.X, 0));
	}
	UpdateLayout();
}

EditorPanel::EditorPanel(Vector3* UIColors, Vector2 Position, Vector2 Scale, Vector2 MinSize, Vector2 MaxSize, bool IsPopup, std::string Title) : UICanvas()
{
	this->UIColors = UIColors;

	Scale.X = std::max(Scale.X, MinSize.X);
	Scale.Y = std::max(Scale.Y, MinSize.Y);
	Scale.X = std::min(Scale.X, MaxSize.X);
	Scale.Y = std::min(Scale.Y, MaxSize.Y);

	this->MinSize = MinSize;
	this->MaxSize = MaxSize;

	TabBackground = new UIBackground(true, Position, UIColors[0], Scale);
	TabBackground->SetBorder(UIBox::E_DARKENED_EDGE, 0.2f);
	TabBackground->HasMouseCollision = true;
	this->Position = Position;
	this->Scale = Scale;
	//TabBackground->IsVisible = false;

	if (IsPopup)
	{
		Position = Position - (Scale * 0.5);
		MainBackground = new UIBox(false, Position);
		MainBackground->AddChild(TabBackground);
		TitleBackground = new UIBackground(true, Position, UIColors[0] * 0.75f, Vector2(Scale.X, 0));
		TitleText = new UIText(0.5f, UIColors[2], Title, Editor::CurrentUI->EngineUIText);
		TitleText->SetPadding(0.005f);
		TitleBackground->AddChild(TitleText);
		MainBackground->AddChild(TitleBackground);
		TabBackground->SetHorizontal(false);
		TabBackground->Align = UIBox::E_REVERSE;
		TabBackground->SetPadding(0);
		TitleBackground->SetPadding(0);
		TitleBackground->HasMouseCollision = true;
		this->Position = Position;
		this->Scale = Scale;
	}
	this->IsPopup = IsPopup;
}

EditorPanel::~EditorPanel()
{
	if (IsPopup)
	{
		delete MainBackground;
	}
	else
	{
		delete TabBackground;
	}
}

void EditorPanel::UpdatePanel()
{
	if ((!dynamic_cast<UIBackground*>(UI::HoveredBox) && !dynamic_cast<UIScrollBox*>(UI::HoveredBox) && !Editor::DraggingTab)
		|| Editor::CurrentUI->DraggedItem || UIScrollBox::IsDraggingScrollBox)
	{
		return;
	}
	Vector2 ClampedMousePosition = Input::MouseLocation;
	if (Editor::DraggingTab && Editor::TabDragHorizontal)
	{
		ClampedMousePosition = Input::MouseLocation.Clamp(Vector2(Editor::DragMinMax.X, 0), Vector2(Editor::DragMinMax.Y, 0));
	}
	else if (Editor::DraggingTab)
	{
		ClampedMousePosition = Input::MouseLocation.Clamp(Vector2(0, Editor::DragMinMax.X), Vector2(0, Editor::DragMinMax.Y));
	}



	Vector2 BorderScale = Vector2(0.01f) * Vector2(1, Graphics::AspectRatio);
	if (!Input::IsLMBDown)
	{
		if (IsDragged)
		{
			Editor::DraggingTab = false;
		}
		IsDragged = false;
		InitialMousePosition = 0;
	}

	if (Math::NearlyEqual(ClampedMousePosition.Y, -1, BorderScale.Y)
		|| Math::NearlyEqual(ClampedMousePosition.Y, 0.95f, BorderScale.Y)
		|| Math::NearlyEqual(ClampedMousePosition.Y, 1, BorderScale.Y))
	{
		return;
	}
	if (Math::NearlyEqual(ClampedMousePosition.X, -1, BorderScale.X) || Math::NearlyEqual(ClampedMousePosition.X, 1, BorderScale.X))
	{
		return;
	}

	if (IsPopup)
	{
		if (MainBackground->IsHovered())
		{
			Editor::PrevHoveringPopup = true;
		}
	}

	bool HorizontalOverride = !IsPopup && !Editor::DraggingPopup && (Editor::DraggingTab && !IsDragged && Editor::TabDragHorizontal
		&& (Math::NearlyEqual(ClampedMousePosition.X, Position.X, 0.02f)
		|| Math::NearlyEqual(ClampedMousePosition.X, Position.X + Scale.X, 0.02f)));

	bool VerticalOverride = !IsPopup && !Editor::DraggingPopup && (Editor::DraggingTab && !IsDragged && !Editor::TabDragHorizontal
		&& (Math::NearlyEqual(ClampedMousePosition.Y, Position.Y, 0.02f)
			|| Math::NearlyEqual(ClampedMousePosition.Y, Position.Y + Scale.Y, 0.02f)));
	if (HorizontalOverride || VerticalOverride || Math::IsPointIn2DBox(Position, Position + Scale, ClampedMousePosition) && !IsDragged)
	{
		if (HorizontalOverride || (!VerticalOverride && !Math::IsPointIn2DBox(Position + BorderScale * Vector2(1, 0),
			Position + Scale - BorderScale * Vector2(1, 0), ClampedMousePosition)))
		{
			if (!HorizontalOverride && IsMouseDown)
			{
				return;
			}
			if (Input::IsLMBDown && !Editor::DraggingPopup)
			{
				IsDragged = true;
				if (!IsPopup)
				{
					Editor::DraggingTab = true;
					Editor::TabDragHorizontal = true;
				}
				IsDragHorizontal = true;
				IsDraggingAll = false;
			}
			Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_WE;
		}
		if (VerticalOverride || (!HorizontalOverride && !Math::IsPointIn2DBox(Position + BorderScale * Vector2(0, 1),
			Position + Scale - BorderScale * Vector2(0, 1), ClampedMousePosition)))
		{
			if (!VerticalOverride && IsMouseDown)
			{
				return;
			}
			if (Input::IsLMBDown && !Editor::DraggingPopup)
			{
				IsDragHorizontal = false;
				if (!IsPopup)
				{
					Editor::DraggingTab = true;
					Editor::TabDragHorizontal = false;
				}
				IsDragged = true;
				IsDraggingAll = false;
			}
			Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_NS;

		}
	}
	if (IsPopup && TitleBackground->IsHovered() && !IsDragged)
	{
		Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_ALL;
		if (Input::IsLMBDown)
		{
			IsDragged = true;
			IsDraggingAll = true;
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
		return;
	}
	else if (IsPopup)
	{
		if (IsDraggingAll)
		{
			Editor::CurrentUI->CurrentCursor = EditorUI::E_RESIZE_ALL;
		}
		else
		{
			Editor::CurrentUI->CurrentCursor = IsDragHorizontal ? EditorUI::E_RESIZE_WE : EditorUI::E_RESIZE_NS;
		}
	}

	if (IsDraggingAll)
	{
		Editor::DraggingPopup = true;
		SetPosition(InitialPosition + (ClampedMousePosition - InitialMousePosition));
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
		if (PositionDifference + Scale.Y > MaxSize.Y) return;

		Position.Y -= PositionDifference;
		SetScale(Vector2(Scale.X, Scale.Y + PositionDifference));
	}

}
#endif