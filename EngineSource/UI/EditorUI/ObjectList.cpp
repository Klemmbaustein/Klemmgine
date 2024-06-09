#if EDITOR
#include "ObjectList.h"
#include <Objects/WorldObject.h>
#include <UI/UIScrollBox.h>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Viewport.h>
#ifdef ENGINE_CSHARP
#include <Objects/CSharpObject.h>
#endif
#include <Engine/Input.h>
#include <Engine/Application.h>

ObjectList::ObjectList(EditorPanel* Parent) : EditorPanel(Parent, "Objects")
{
	ObjectListBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	PanelMainBackground
		->AddChild(ObjectListBox);
}

void ObjectList::Tick()
{
	TickPanel();
	if (Objects::AllObjects.size() != ObjectSize)
	{
		ObjectSize = Objects::AllObjects.size();
		OnResized();
	}
}

void ObjectList::OnButtonClicked(int Index)
{
	HandlePanelButtons(Index);
	if (Index >= 0)
	{
		if (!Input::IsKeyDown(Input::Key::LSHIFT))
		{
			for (auto o : Objects::AllObjects)
			{
				o->IsSelected = false;
			};
		}
		Objects::AllObjects[Index]->IsSelected = true;
		OnResized();
	}
	if (Index < 0 && Index > -1000)
	{
		std::string Elem = "OBJ_CAT_" + Application::EditorInstance->ObjectCategories[-Index - 1];
		if (Application::EditorInstance->CollapsedItems.contains(Elem))
		{
			Application::EditorInstance->CollapsedItems.erase(Elem);
		}
		else
		{
			Application::EditorInstance->CollapsedItems.insert(Elem);
		}
		OnResized();
	}
}

void ObjectList::OnResized()
{
	ObjectListBox
		->SetPadding(0.005f, 0.005f, 0.005f / Graphics::AspectRatio, 0.005f / Graphics::AspectRatio)
		->SetMinSize(Vector2(Scale.X - (0.01f / Graphics::AspectRatio), Scale.Y - 0.01f))
		->SetMaxSize(Vector2(Scale.X - (0.01f / Graphics::AspectRatio), Scale.Y - 0.01f));

	ObjectListBox->DeleteChildren();
	ObjectListBox->SetVerticalAlign(UIBox::Align::Reverse);
	ListIterator = 0;

	auto ObjectList = Application::EditorInstance->GetObjectList();
	GenerateObjectListSection(ObjectList, 0);
}

void ObjectList::GenerateObjectListSection(std::vector<EditorUI::ObjectListItem> Section, float Depth)
{
	for (auto& Object : Section)
	{
		UIButton* ElementButton = (new UIButton(UIBox::Orientation::Horizontal,
			0,
			(ListIterator++ % 2) ? EditorUI::UIColors[0] : Vector3::Lerp(EditorUI::UIColors[0], 0.5f, 0.2f),
			this,
			Object.Object ? Object.ListIndex : -1 - Object.ListIndex));

		bool IsCSObj = false;
#if ENGINE_CSHARP
		IsCSObj = Object.Object && Object.Object->GetObjectDescription().ID == CSharpObject::GetID();
		Vector3 ObjectColor = IsCSObj ? EditorUI::ItemColors.at("cs") : EditorUI::ItemColors.at("cpp");
#else
		Vector3 ObjectColor = Editor::ItemColors["cpp"];
#endif

		unsigned int ElemIcon = 5;

		if (Object.Object)
		{
			ElemIcon = 0;
			if (IsCSObj)
			{
				ElemIcon = 22;
			}
		}
		if (Object.IsScene)
		{
			ElemIcon = 7;
		}

		ObjectListBox->AddChild(ElementButton
			->SetVerticalAlign(UIBox::Align::Centered)
			->SetMinSize(Vector2(Scale.X - 0.005f, 0.045f))
			->SetPadding(0, 0, 0, 0.01f));

		if (!Object.Object && !Object.IsScene)
		{
			ElementButton->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], 0.03f))
				->SetUseTexture(true, Application::EditorInstance->Textures[13ull + (size_t)Object.IsCollapsed])
				->SetPadding(0, 0.01f, Depth - 0.04f / Graphics::AspectRatio + 0.005f, 0.001f)
				->SetSizeMode(UIBox::SizeMode::AspectRelative));
		}

		auto Icon = (new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], 0.04f))
			->SetUseTexture(true, Application::EditorInstance->Textures[ElemIcon])
			->SetPadding(0, 0, 0, 0.005f)
			->SetSizeMode(UIBox::SizeMode::AspectRelative);
		ElementButton->AddChild(Icon);

		if (Object.Object)
		{
			Icon->SetPadding(0, 0, Depth, 0.005f);
		}

		ElementButton->AddChild((new UIText(0.4f, EditorUI::UIColors[2], Object.Name, EditorUI::Text))
			->SetPadding(0.005f, 0.005f, 0, 0));

		if (Object.IsSelected)
		{
			ElementButton->SetColor(ElementButton->GetColor() * 3 * Vector3(1, 0.75f, 0));
		}

		if (Object.Object)
		{
			std::string ClassName;
#if ENGINE_CSHARP
			if (IsCSObj)
			{
				ClassName = static_cast<CSharpObject*>(Object.Object)->CSharpClass;
			}
			else
			{
				ClassName = Object.Object->GetObjectDescription().Name;
			}
#else
			ClassName = "Class " + Object.Object->GetObjectDescription().Name;
#endif
		}
		if (!Object.Object && !Object.IsCollapsed)
		{
			GenerateObjectListSection(Object.Children, Depth + 0.02f);
		}
	}
}
#endif