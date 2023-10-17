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

ObjectList::ObjectList(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, Vector2(0.2f, 0.5f), Vector2(0.8f, 1.25f))
{
	TabBackground->SetHorizontal(false);
	ObjectListBox = new UIScrollBox(false, 0, true);
	HeaderBox = new UIBackground(false, 0, Vector3(UIColors[0]), 0);
	HeaderBox->SetBorder(UIBox::BorderType::DarkenedEdge, 0.25);
	ObjectListBox
		->SetPadding(0.005f, 0.005f, 0.005f / Graphics::AspectRatio, 0.005f / Graphics::AspectRatio)
		->SetMinSize(Vector2(Scale.X - (0.005f / Graphics::AspectRatio), Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y - 0.01f))
		->SetMaxSize(Vector2(Scale.X - (0.005f / Graphics::AspectRatio), Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y - 0.01f));
	TabBackground->SetAlign(UIBox::Align::Reverse);
	TabBackground
		->AddChild(HeaderBox
			->SetPadding(0)
			->SetMinSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y))
			->SetMaxSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y)))
		->AddChild(ObjectListBox);
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
		if (!Input::IsKeyDown(Input::Key::LSHIFT))
		{
			for (auto o : Objects::AllObjects)
			{
				o->IsSelected = false;
			};
		}
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
		->SetPadding(0.005f, 0.005f, 0.005f / Graphics::AspectRatio, 0.005f / Graphics::AspectRatio)
		->SetMinSize(Vector2(Scale.X - (0.005f / Graphics::AspectRatio), Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y - 0.01f))
		->SetMaxSize(Vector2(Scale.X - (0.005f / Graphics::AspectRatio), Scale.Y - Editor::CurrentUI->UIElements[2]->Scale.Y - 0.01f));
	HeaderBox
		->SetMinSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y))
		->SetMaxSize(Vector2(Scale.X, Editor::CurrentUI->UIElements[2]->Scale.Y));

	ObjectListBox->DeleteChildren();
	ObjectListBox->BoxAlign = UIBox::Align::Reverse;
	ListIterator = 0;

	auto ObjectList = Editor::CurrentUI->GetObjectList();
	GenerateObjectListSection(ObjectList, 0);
}

void ObjectList::GenerateObjectListSection(std::vector<EditorUI::ObjectListItem> Section, float Depth)
{
	TextRenderer* Renderer = Editor::CurrentUI->EngineUIText;

	for (auto& Object : Section)
	{
		UIButton* ElementButton = (new UIButton(true,
			0,
			(ListIterator++ % 2) ? UIColors[0] : Vector3::Lerp(UIColors[0], 0.5f, 0.2f),
			this,
			Object.Object ? Object.ListIndex : -1 - Object.ListIndex));

		bool IsCSObj = false;
#if ENGINE_CSHARP
		IsCSObj = Object.Object && Object.Object->GetObjectDescription().ID == CSharpObject::GetID();
		Vector3 ObjectColor = IsCSObj ? Editor::ItemColors.at("cs") : Editor::ItemColors.at("cpp");
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
			->SetMinSize(Vector2(TabBackground->GetUsedSize().X - 0.02f, 0.045f))
			->SetPadding(0, 0, 0, 0.01f));

		if (!Object.Object && !Object.IsScene)
		{
			ElementButton->AddChild((new UIBackground(true, 0, UIColors[2], 0.03f))
				->SetUseTexture(true, Editor::CurrentUI->Textures[13ull + (size_t)Object.IsCollapsed])
				->SetPadding(0, 0.01f, Depth - 0.04f / Graphics::AspectRatio + 0.005f, 0.001f)
				->SetSizeMode(UIBox::SizeMode::PixelRelative));
		}

		auto Icon = (new UIBackground(true, 0, UIColors[2], 0.04f))
			->SetUseTexture(true, Editor::CurrentUI->Textures[ElemIcon])
			->SetPadding(0, 0, 0, 0.005f)
			->SetSizeMode(UIBox::SizeMode::PixelRelative);
		ElementButton->AddChild(Icon);

		if (Object.Object)
		{
			Icon->SetPadding(0, 0, Depth, 0.005f);
		}

		ElementButton->AddChild((new UIText(0.4f, UIColors[2], Object.Name, Renderer))
			->SetPadding(0.005f, 0.005f, 0, 0));

		auto v = dynamic_cast<Viewport*>(Editor::CurrentUI->UIElements[4]);
		if (Object.IsSelected)
		{
			ElementButton->SetColor(ElementButton->GetColor() * 3 * Vector3(1, 0.75f, 0));
			ElementButton->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);
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