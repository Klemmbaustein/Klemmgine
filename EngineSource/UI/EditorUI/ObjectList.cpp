#if EDITOR
#include "ObjectList.h"
#include <Objects/WorldObject.h>
#include <UI/UIScrollBox.h>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Viewport.h>


ObjectList::ObjectList(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.2, 0.5), Vector2(0.8, 1.25))
{
	TabBackground->SetHorizontal(false);
	ObjectListBox = new UIScrollBox(false, 0, 50);
	HeaderBox = new UIBackground(false, 0, Vector3(UIColors[0]), 0);
	HeaderBox->SetBorder(UIBox::E_DARKENED_EDGE, 0.25);
	ObjectListBox
		->SetPosition(TabBackground->GetPosition())
		->SetPadding(0)
		->SetMinSize(Vector2(Scale.X, Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y))
		->SetMaxSize(Vector2(Scale.X, Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y));
	TabBackground->Align = UIBox::E_REVERSE;
	TabBackground->AddChild(HeaderBox
		->SetPadding(0)
		->SetMinSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y))
		->SetMaxSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y)));
}

void ObjectList::Tick()
{
	UpdatePanel();
	if (Objects::AllObjects.size() != ObjectSize)
	{
		ObjectSize = Objects::AllObjects.size();
		UpdateLayout();
	}
}

void ObjectList::OnButtonClicked(int Index)
{
	if (Index >= 0)
	{
		for (auto o : Objects::AllObjects)
		{
			o->IsSelected = false;
		};
		Objects::AllObjects[Index]->IsSelected = true;
	}
	if (Index < 0)
	{
		std::string Elem = "OBJ_CAT_" + Editor::CurrentUI->ObjectCategories[-Index - 1];
		if (Editor::CurrentUI->CollapsedItems.contains(Elem))
		{
			Editor::CurrentUI->CollapsedItems.erase(Elem);
		}
		else
		{
			Editor::CurrentUI->CollapsedItems.insert(Elem);
		}
		UpdateLayout();
	}
}

void ObjectList::UpdateLayout()
{
	ObjectListBox
		->SetPosition(TabBackground->GetPosition())
		->SetMinSize(Vector2(Scale.X, Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y))
		->SetMaxSize(Vector2(Scale.X, Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y));
	HeaderBox
		->SetMinSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y))
		->SetMaxSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y));

	ObjectListBox->DeleteChildren();
	ObjectListBox->Align = UIBox::E_REVERSE;

	auto ObjectList = Editor::CurrentUI->GetObjectList();
	GenerateObjectListSection(ObjectList, 0);
}

void ObjectList::Save()
{
}

void ObjectList::Load(std::string File)
{
}


void ObjectList::GenerateObjectListSection(std::vector<EditorUI::ObjectListItem> Section, float Depth)
{
	for (auto& Object : Section)
	{
		/*
		* ----------------------
		* | Co |               |
		* | lo | Object Name   |
		* | r  |               |
		* ----------------------
		*/
		auto v = dynamic_cast<Viewport*>(Editor::CurrentUI->UIElements[4]);

		auto NewListEntryBackground = new UIButton(true, 0, UIColors[1], this, Object.Object ? Object.ListIndex : -1 - Object.ListIndex);
		NewListEntryBackground->SetBorder(UIBox::E_ROUNDED, 0.8);
		if (v && v->SelectedObjects.size() && Object.Object == v->SelectedObjects[0])
		{
			NewListEntryBackground->SetColor(UIColors[1] * 2);
		}
		NewListEntryBackground->SetMinSize(Vector2(Scale.X / 1.2 - Depth, 0));
		NewListEntryBackground->SetPadding(0.01, 0.0, 0.01 + Depth, 0.01);
		auto ListEntryObjectColor = new UIBackground(true, 0, Vector3(0.5, 0.5, 0.5), Vector2(0.01, 0.05));
		ListEntryObjectColor->SetTryFill(true);
		ListEntryObjectColor->SetPadding(0);
		NewListEntryBackground->AddChild(ListEntryObjectColor);
		auto TextBox = new UIBox(false, 0);

		if (!Object.Object)
		{
			auto CollapsedArrow = new UIBackground(true, Vector2(0), 1, Vector2(0.05));
			CollapsedArrow->SetPadding(0, 0, 0.01, 0);
			CollapsedArrow->SetSizeMode(UIBox::E_PIXEL_RELATIVE);
			CollapsedArrow->SetUseTexture(true, Object.IsCollapsed ? Editor::CurrentUI->Textures[14] : Editor::CurrentUI->Textures[13]);
			//CollapsedArrow->SetTryFill(true);
			NewListEntryBackground->AddChild(CollapsedArrow);
		}

		TextBox->SetPadding(0);
		NewListEntryBackground->AddChild(TextBox);
		UIText* ListEntryText;
		if (Object.Object)
		{
			ListEntryObjectColor->SetColor(0.5);
			ListEntryText = new UIText(0.3, 0.7, "Class " + Object.Object->GetObjectDescription().Name, Editor::CurrentUI->EngineUIText);
		}
		else
		{
			ListEntryObjectColor->SetColor(Editor::ItemColors["dir"]);
			ListEntryText = new UIText(0.3, 0.7, Object.IsScene ? "Scene" : "Category", Editor::CurrentUI->EngineUIText);
		}
		ListEntryText->SetPadding(0.005, 0.005, 0.01, 0.01);
		ListEntryText->Wrap = true;
		ListEntryText->WrapDistance = 0.3;
		TextBox->AddChild(ListEntryText);
		ListEntryText = new UIText(0.4, 1, Object.Name, Editor::CurrentUI->EngineUIText);
		ListEntryText->SetPadding(0.0, 0, 0.01, 0.01);
		ListEntryText->Wrap = true;
		ListEntryText->WrapDistance = 0.5;
		TextBox->AddChild(ListEntryText);
		ObjectListBox->AddChild(NewListEntryBackground);
		if (!Object.Object)
		{
			GenerateObjectListSection(Object.Children, Depth + 0.01);
		}
	}
}
#endif