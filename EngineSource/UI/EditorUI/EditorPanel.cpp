#if EDITOR
#include "EditorPanel.h"
#include <Math/Math.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Engine/Application.h>
#include <Engine/EngineError.h>

bool EditorPanel::IsDraggingHorizontal = true;
float EditorPanel::StartPosition = 0.0f;
EditorPanel* EditorPanel::Dragged = nullptr;

static EditorPanel* DropdownPanel = nullptr;
static EditorPanel* DraggedTabPanel = nullptr;
static UIBackground* PreviewBackground = nullptr;

void EditorPanel::Collapse()
{
	if (Parent && Parent->Children.size() > 1 && Parent->ChildrenAlign == ChildrenType::Vertical)
	{
		Collapsed = !Collapsed;
		GetAbsoluteParent()->OnPanelResized();
	}
}

void EditorPanel::ClearParent(bool Delete)
{
	if (Parent)
	{
		for (size_t i = 0; i < Parent->Children.size(); i++)
		{
			if (Parent->Children[i] == this)
			{
				Parent->Children.erase(Parent->Children.begin() + i);
				if (Parent->ActiveTab > i && Parent->ActiveTab > 0)
				{
					Parent->ActiveTab--;
				}
				if (Parent->ActiveTab >= Parent->Children.size())
				{
					Parent->ActiveTab = Parent->Children.size() - 1;
				}
				break;
			}
		}

		if (Parent->Children.empty() && Delete)
		{
			delete Parent;
		}

		Parent->GetAbsoluteParent()->OnPanelResized();	
	}
}

void EditorPanel::ReSort()
{
	if (Parent)
	{
		PanelMainBackground->SetRenderOrderIndex(Parent->PanelMainBackground->GetRenderOrderIndex() + 1);
	}
	for (EditorPanel* c : Children)
	{
		c->ReSort();
	}
}

void EditorPanel::TickPanels()
{
	if (DraggedTabPanel)
	{
		HandleDrag();
	}
}

EditorPanel* EditorPanel::GetAbsoluteParent()
{
	if (Parent) return Parent->GetAbsoluteParent();
	return this;
}

EditorPanel::EditorPanel(Vector2 Position, Vector2 Scale, std::string Name) : EditorPanel(nullptr, Name)
{
	this->Position = Position;
	this->Scale = Scale;
	PanelMainBackground->SetPosition(Position);
	PanelMainBackground->SetMinSize(Scale);
	PanelMainBackground->SetMaxSize(Scale);
}

EditorPanel::EditorPanel(EditorPanel* NewParent, std::string Name, size_t TabPosition)
{
	ENGINE_ASSERT(!Name.empty(), "Name should not be empty");
	this->Name = Name;
	Parent = NewParent;
	PanelMainBackground = new UIBackground(UIBox::Orientation::Vertical, 0, EditorUI::UIColors[0], 1);
	TabList = new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 0.75, Vector2(1, 0.04f));
	PanelMainBackground
		->SetPadding(0)
		->SetBorder(UIBox::BorderType::DarkenedEdge, 0.15f);
	PanelMainBackground->AddChild(TabList
		->SetPadding(0));
	PanelMainBackground->HasMouseCollision = true;
	TabList->HasMouseCollision = true;
	if (Parent)
	{
		if (TabPosition != SIZE_MAX)
		{
			Parent->Children.insert(Parent->Children.begin() + TabPosition, this);
			Parent->UpdateTabs();
		}
		else
		{
			Parent->Children.push_back(this);
			Parent->UpdateTabs();
		}
	}
	UpdatePanel();
}

void EditorPanel::HandlePanelButtons(int Index)
{
	if (Index == -10000)
	{
		if (Tabs[0] != UI::HoveredBox)
		{
			delete this;
			return;
		}
		Collapse();
	}
}

void EditorPanel::HandlePanelDrag(int Index)
{
	if (Index == -10000)
	{
		DraggedTabPanel = this;	
	}

}

void EditorPanel::OnButtonClicked(int Index)
{
	if (Tabs[Index] != UI::HoveredBox)
	{
		delete Children[Index];
		return;
	}

	if (Index == ActiveTab || Collapsed)
	{
		ActiveTab = (size_t)Index;
		Collapse();
	}
	else
	{
		ActiveTab = (size_t)Index;
		UpdateTabs();
	}
}

void EditorPanel::OnButtonDragged(int Index)
{
	HandlePanelDrag(Index);
	if (Index >= 0)
	{
		DraggedTabPanel = Children[Index];
	}
}

void EditorPanel::UpdatePanel()
{
	if (Parent)
	{
		PanelMainBackground->SetPosition(Position);
		PanelMainBackground->SetMinSize(Scale);
		PanelMainBackground->SetMaxSize(Scale);
		UpdateTabs();
	}
}

void EditorPanel::Tick()
{
	TickPanel();
}

void EditorPanel::TickPanelInternal()
{
	if (DraggedTabPanel)
	{
		Dragged = nullptr;
	}

	if (Input::IsRMBClicked && UI::HoveredBox && (UI::HoveredBox->IsChildOf(TabList) || (Parent && UI::HoveredBox->IsChildOf(Parent->TabList))))
	{
		std::vector<EditorUI::DropdownItem> OptionsList;
		if (Parent && Parent->Children.size() > 1 && Parent->ChildrenAlign == ChildrenType::Vertical)
		{
			OptionsList.push_back(EditorUI::DropdownItem(Collapsed ? "Expand" : "Collapse", []() {
				DropdownPanel->Collapse();
				}));
		}
		if (CanBeClosed)
		{
			OptionsList.push_back(EditorUI::DropdownItem("Close", []() {
				delete DropdownPanel;
				DropdownPanel = nullptr; 
				}));
		}

		if (!OptionsList.empty())
		{
			DropdownPanel = this;
			Application::EditorInstance->ShowDropdownMenu(
				{
					OptionsList
				}, Input::MouseLocation);
		}
	}

	if (Collapsed)
	{
		return;
	}
	if (Math::NearlyEqual(Position.X + PanelMainBackground->GetUsedSize().X, Input::MouseLocation.X, 0.01f)
		&& Math::IsPointIn2DBox(Vector2(-1, Position.Y), Vector2(1, Position.Y + PanelMainBackground->GetUsedSize().Y), Input::MouseLocation)
		&& !Dragged
		&& Parent
		&& Parent->ChildrenAlign == ChildrenType::Horizontal)

	{
		EditorUI::CurrentCursor = EditorUI::CursorType::Resize_WE;
		if (Input::IsLMBClicked)
		{
			StartPosition = Input::MouseLocation.X;
			Dragged = this;
			IsDraggingHorizontal = true;
		}
	}

	if (Math::NearlyEqual(Position.Y + PanelMainBackground->GetUsedSize().Y, Input::MouseLocation.Y, 0.01f)
		&& Math::IsPointIn2DBox(Vector2(Position.X, -1), Vector2(Position.X + PanelMainBackground->GetUsedSize().X, 1), Input::MouseLocation)
		&& !Dragged
		&& Parent
		&& Parent->ChildrenAlign == ChildrenType::Vertical)
	{
		EditorUI::CurrentCursor = EditorUI::CursorType::Resize_NS;
		if (Input::IsLMBClicked)
		{
			StartPosition = Input::MouseLocation.Y;
			Dragged = this;
			IsDraggingHorizontal = false;
		}
	}

	if (Dragged && IsDraggingHorizontal)
	{
		EditorUI::CurrentCursor = EditorUI::CursorType::Resize_WE;
	}
	if (Dragged && !IsDraggingHorizontal)
	{
		EditorUI::CurrentCursor = EditorUI::CursorType::Resize_NS;
	}

	if (!Input::IsLMBDown && Dragged == this)
	{
		Dragged = nullptr;
		float Difference = 0;
		if (IsDraggingHorizontal)
		{
			Difference = Input::MouseLocation.X - StartPosition;
		}
		else
		{
			Difference = Input::MouseLocation.Y - StartPosition;
		}
		if (Size + Difference < 0)
		{
			return;
		}

		Size += Difference;
		bool FoundSelf = false;

		for (size_t i = 0; i < Parent->Children.size(); i++)
		{
			if (FoundSelf)
			{
				Parent->Children[i]->Size -= Difference;
				break;
			}
			if (Parent->Children[i] == this)
			{
				FoundSelf = true;
			}
		}

		GetAbsoluteParent()->OnPanelResized();
	}

	for (EditorPanel* c : Children)
	{
		c->TickPanelInternal();
	}
}

void EditorPanel::OnPanelResized()
{
	UpdateTabs();
	PanelMainBackground->UpdateSelfAndChildren();
	for (const auto& c : Children)
	{
		c->OnPanelResized();
	}
	OnResized();
}

void EditorPanel::OnResized()
{
}

void EditorPanel::AddTab(EditorPanel* NewTab, ChildrenType Align, size_t TabPosition)
{
	if (NewTab == this)
	{
		return;
	}
	if (typeid(*this) == typeid(EditorPanel) && Align == ChildrenType::Tabs)
	{
		if (NewTab->Parent)
		{
			NewTab->ClearParent(false);
		}
		NewTab->Parent = this;
		if (TabPosition < Children.size())
		{
			Children.insert(Children.begin() + TabPosition, NewTab);
			UpdateTabs();
		}
		else
		{
			Children.push_back(NewTab);
			UpdateTabs();
		}
		ActiveTab = std::min(TabPosition, Children.size() - 1);
		GetAbsoluteParent()->ReSort();
		GetAbsoluteParent()->OnPanelResized();
		return;
	}
	size_t i = 0;

	for (i = 0; i < Parent->Children.size(); i++)
	{
		if (Parent->Children[i] == this)
		{
			break;
		}
	}

	if (Align == ChildrenType::Tabs && Parent && Parent->ChildrenAlign == ChildrenType::Tabs)
	{
		Parent->AddTab(NewTab, ChildrenType::Tabs, i);
	}
	else
	{
		EditorPanel* OldParent = Parent;
		ClearParent(false);
		OldParent->OnPanelResized();

		auto NewPanel = new EditorPanel(OldParent, "panel", i);
		NewPanel->ChildrenAlign = Align;
		NewPanel->Size = Size;

		if (TabPosition != 0)
		{
			NewPanel->AddTab(this);
		}
		NewPanel->AddTab(NewTab);
		if (TabPosition == 0)
		{
			NewPanel->AddTab(this);
		}
		Parent->ActiveTab = std::min(TabPosition, Parent->Children.size() - 1);
		NewTab->Size = NewPanel->Scale.Y / 2;
		Size = NewPanel->Size / 2;
		GetAbsoluteParent()->ReSort();
		GetAbsoluteParent()->OnPanelResized();
	}
}
void EditorPanel::AddTabButton(bool Selected, int Index, std::string Name, bool Closable)
{
	UIButton* TabButton = new UIButton(UIBox::Orientation::Vertical, 0, Selected ? EditorUI::UIColors[0] : EditorUI::UIColors[0] * 0.75f, this, Index);
	UIBox* HorizontalBox = new UIBox(UIBox::Orientation::Horizontal, 0);
	Tabs.push_back(TabButton);
	TabList->AddChild(TabButton
		->SetCanBeDragged(true)
		->SetPadding(0, 0, 0.005f, 0.005f)
		->SetMinSize(Vector2(0.05f, 0.04f))
		->SetPaddingSizeMode(UIBox::SizeMode::PixelRelative)
		->AddChild((new UIBackground(UIBox::Orientation::Horizontal,
			0,
			Selected ? EditorUI::UIColors[2] : EditorUI::UIColors[0] * 0.75f,
			Vector2(0.02f, 2.0f / Graphics::WindowResolution.Y)))
			->SetTryFill(true)
			->SetPadding(0))
		->AddChild(HorizontalBox
			->AddChild((new UIText(0.45f, EditorUI::UIColors[2], Name, EditorUI::Text))
			->SetPadding(0.005f))));
	HorizontalBox
		->SetMaxSize(Vector2(2, 0.04f))
		->SetPadding(0)
		->SetVerticalAlign(UIBox::Align::Centered);
	if (Closable)
	{
		HorizontalBox->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, Index))
			->SetUseTexture(true, EditorUI::Textures[4])
			->SetMinSize(0.03f)
			->SetSizeMode(UIBox::SizeMode::PixelRelative)
			->SetPadding(0, 0, 0, 0.005f));
	}
}

void EditorPanel::HandleDrag()
{
	if (!PreviewBackground)
	{
		PreviewBackground = new UIBackground(UIBox::Orientation::Horizontal, 0, Vector3(0.2f, 0.8f, 1.0f), 0);
		PreviewBackground
			->SetOpacity(0.5f)
			->SetBorder(UIBox::BorderType::DarkenedEdge, 0.15f);
	}

	for (UICanvas* c : Graphics::UIToRender)
	{
		EditorPanel* Panel = dynamic_cast<EditorPanel*>(c);
		if (!Panel || !UI::HoveredBox)
		{
			continue;
		}
		if (UI::HoveredBox->IsChildOf(Panel->TabList) || UI::HoveredBox == Panel->TabList)
		{
			size_t Index = 0;
			Vector2 Position = Panel->Tabs[0]->GetPosition();
			for (UIButton* b : Panel->Tabs)
			{
				if (b->GetPosition().X + (b->GetUsedSize().X / 2.0f) > Input::MouseLocation.X)
				{
					break;
				}
				Position = b->GetPosition() + Vector2(b->GetUsedSize().X, 0);
				Index++;
			}
			PreviewBackground
				->SetPosition(Position)
				->SetMinSize(Vector2(6.0f / Graphics::WindowResolution.X, 0.045f));
			if (!Input::IsLMBDown)
			{
				Panel->AddTab(DraggedTabPanel, ChildrenType::Tabs, Index);
			}
			break;
		}
		if (UI::HoveredBox->IsChildOf(Panel->PanelMainBackground) || UI::HoveredBox == Panel->PanelMainBackground)
		{
			ChildrenType Align = ChildrenType::Tabs;
			Vector2 Scale = Panel->Scale;
			size_t Index = SIZE_MAX;

			Vector2 Pos = Panel->PanelMainBackground->GetPosition();

			if (Math::IsPointIn2DBox(Pos, Pos + Scale * Vector2(1.0f, 0.25f), Input::MouseLocation))
			{
				Scale *= Vector2(1.0f, 0.5f);
				Align = ChildrenType::Vertical;
				Index = 0;
			}
			else if (Math::IsPointIn2DBox(Pos + Scale * Vector2(0.0f, 0.75f), Pos + Scale, Input::MouseLocation))
			{
				Pos += Scale * Vector2(0.0f, 0.5f);
				Scale *= Vector2(1.0f, 0.5f);
				Align = ChildrenType::Vertical;
				Index = SIZE_MAX;
			}
			else if (Math::IsPointIn2DBox(Pos, Pos + Scale * Vector2(0.25f, 1.0f), Input::MouseLocation))
			{
				Scale *= Vector2(0.5f, 1.0f);
				Align = ChildrenType::Horizontal;
				Index = 0;
			}
			else if (Math::IsPointIn2DBox(Pos + Scale * Vector2(0.75f, 0), Pos + Scale, Input::MouseLocation))
			{
				Pos += Scale * Vector2(0.5f, 0.0f);
				Scale *= Vector2(0.5f, 1.0f);
				Align = ChildrenType::Horizontal;
				Index = SIZE_MAX;
			}


			PreviewBackground
				->SetPosition(Pos)
				->SetMinSize(Scale);
			if (!Input::IsLMBDown)
			{
				if (Panel->Parent->ChildrenAlign == ChildrenType::Tabs)
				{
					Panel = Panel->Parent;
				}

				Panel->AddTab(DraggedTabPanel, Align, Index);
			}
		}
	}
	if (!Input::IsLMBDown)
	{
		DraggedTabPanel = nullptr;
		delete PreviewBackground;
		PreviewBackground = nullptr;
	}
}

void EditorPanel::TickPanel()
{
	if (Parent)
	{
		return;
	}

	for (EditorPanel* c : Children)
	{
		c->TickPanelInternal();
	}
}

void EditorPanel::OnItemDropped(DroppedItem Info)
{

}

void EditorPanel::UpdateTabs()
{
	Tabs.clear();
	TabList->DeleteChildren();
	if (Parent && Parent->ChildrenAlign == ChildrenType::Tabs)
	{
		TabList->SetMinSize(0);
		return;
	}

	PanelMainBackground->IsVisible = Children.empty() || ChildrenAlign == ChildrenType::Tabs;
	PanelMainBackground->SetOpacity(Children.empty() && BackgroundVisible ? 1.0f : 0.0f);
	if (ChildrenAlign == ChildrenType::Tabs)
	{
		TabList->SetMinSize(Vector2(PanelMainBackground->GetMinSize().X, 0.05f));
		TabList->SetColor(EditorUI::UIColors[0] * 0.75);

		if (Children.empty())
		{
			Scale = Scale - Vector2(0, 0.025f);
			AddTabButton(!Collapsed, -10000, Name, CanBeClosed);
		}
		for (size_t i = 0; i < Children.size(); i++)
		{
			AddTabButton((i == ActiveTab) && !Collapsed, (int)i, Children[i]->Name, Children[i]->CanBeClosed);
			Children[i]->Position = Position;
			Children[i]->Scale = Scale - Vector2(0, 0.05f);

			auto BackgroundChildren = Children[i]->PanelMainBackground->GetChildren();
			for (auto& c : BackgroundChildren)
			{
				if (c != Children[i]->TabList)
				{
					c->IsVisible = !Children[i]->Collapsed && !Collapsed;
				}
			}
			Children[i]->PanelMainBackground->IsVisible = ActiveTab == i;
			Children[i]->UpdatePanel();
		}

	}
	if (ChildrenAlign == ChildrenType::Vertical)
	{
		TabList->SetMinSize(0);
		float Pos = 0;
		float NextSize = 0;
		float MaxSize = Scale.Y;
		for (size_t i = 0; i < Children.size(); i++)
		{
			Children[i]->Position = Position + Vector2(0, Pos);
			float ChldSize = Children[i]->Size;
			if (Children[i]->Collapsed)
			{
				NextSize += ChldSize - 0.05f;
				ChldSize = 0.05f;
			}
			else
			{
				ChldSize += NextSize;
				NextSize = 0;
			}

			if (i == Children.size() - 2 && Children[Children.size() - 1]->Collapsed)
			{
				ChldSize += MaxSize - (Pos + ChldSize) - 0.05f;
			}

			if (i == Children.size() - 1)
			{
				Children[i]->Scale = Vector2(Scale.X, MaxSize - Pos);
			}
			else
			{
				Children[i]->Scale = Vector2(Scale.X, ChldSize);
			}

			auto BackgroundChildren = Children[i]->PanelMainBackground->GetChildren();
			for (auto& c : BackgroundChildren)
			{
				if (c != Children[i]->TabList)
				{
					c->IsVisible = !Children[i]->Collapsed && !Collapsed;
				}
			}

			Pos += ChldSize;
			Children[i]->UpdatePanel();
			Children[i]->OnResized();
		}
	}
	if (ChildrenAlign == ChildrenType::Horizontal)
	{
		TabList->SetMinSize(0);
		float Pos = 0;
		float MaxSize = Scale.X;
		for (size_t i = 0; i < Children.size(); i++)
		{
			Children[i]->Position = Position + Vector2(Pos, 0);
			if (i == Children.size() - 1)
			{
				Children[i]->Scale = Vector2(MaxSize - Pos, Scale.Y);
			}
			else
			{
				Children[i]->Scale = Vector2(Children[i]->Size, Scale.Y);
			}
			Pos += Children[i]->Size;
			Children[i]->UpdatePanel();
			Children[i]->OnResized();
		}
	}

}

EditorPanel::~EditorPanel()
{
	ClearParent(true);
	if (Parent)
	{
		for (size_t i = 0; i < Parent->Children.size(); i++)
		{
			if (Parent->Children[i] == this)
			{
				if (i > 0)
				{
					Parent->Children[i - 1]->Size += Size;
				}
				break;
			}
		}
	}
	delete PanelMainBackground;
}

#endif