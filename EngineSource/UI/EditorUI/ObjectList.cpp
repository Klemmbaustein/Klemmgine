#if EDITOR
#include "ObjectList.h"
#include <Objects/SceneObject.h>
#include <UI/UIScrollBox.h>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <Engine/Log.h>
#include <UI/EditorUI/EditorUI.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/EditorUI/EditorDropdown.h>
#include <Engine/Utility/FileUtility.h>
#ifdef ENGINE_CSHARP
#include <Objects/CSharpObject.h>
#endif
#include <Engine/Input.h>
#include <Engine/Application.h>

ObjectList::ObjectList(EditorPanel* Parent) : EditorPanel(Parent, "Objects", "object_list")
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

	if (Input::IsRMBClicked && PanelMainBackground->IsHovered() && EditorUI::SelectedObjects.size())
	{
		new EditorDropdown({
			EditorDropdown::DropdownItem{
			.Title = "Duplicate",
			.OnPressed = []() {
				Viewport::CopySelectedObjects();
				}
			},
			EditorDropdown::DropdownItem{
			.Title = "Delete",
			.OnPressed = []() {
				for (SceneObject* i : EditorUI::SelectedObjects)
				{
					Objects::DestroyObject(i);
				}
				EditorUI::ChangedScene = true;
				}
			},
			}, Input::MouseLocation);
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
		std::string Elem = "OBJ_CAT_" + ObjectCategories[-Index - 1];
		if (CollapsedItems.contains(Elem))
		{
			CollapsedItems.erase(Elem);
		}
		else
		{
			CollapsedItems.insert(Elem);
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

	auto ObjectList = GetObjectList();
	GenerateObjectListSection(ObjectList, 0);
}

void ObjectList::GenerateObjectListSection(std::vector<ObjectListItem> Section, float Depth)
{
	for (auto& Object : Section)
	{
		UIButton* ElementButton = (new UIButton(UIBox::Orientation::Horizontal,
			0,
			(ListIterator++ % 2) ? EditorUI::UIColors[0] : Vector3::Lerp(EditorUI::UIColors[0], 0.5f, 0.2f),
			this,
			Object.Object ? Object.ListIndex : -1 - Object.ListIndex));

		bool IsCSObj = false;

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


std::vector<ObjectList::ObjectListItem> ObjectList::GetObjectList()
{
	std::vector<ObjectListItem> ObjectList;
	ObjectCategories.clear();
	size_t ListIndex = 0;
	for (SceneObject* o : Objects::AllObjects)
	{
		ObjectListItem* SceneList = nullptr;
		// Get the list for the scene the object belongs to
		for (auto& item : ObjectList)
		{
			if (item.Name == FileUtil::GetFileNameFromPath(o->CurrentScene))
			{
				SceneList = &item;
			}
		}

		if (!SceneList)
		{
			std::string SceneName = FileUtil::GetFileNameFromPath(o->CurrentScene);
			ObjectList.push_back(ObjectListItem(SceneName, {}, true, CollapsedItems.contains("OBJ_CAT_" + SceneName)));
			ObjectCategories.push_back(FileUtil::GetFileNameFromPath(o->CurrentScene));
			ObjectList[ObjectList.size() - 1].ListIndex = (int)ObjectCategories.size() - 1;
			SceneList = &ObjectList[ObjectList.size() - 1];
		}

		// Separate the Object's category into multiple strings
		std::string CurrentPath = Objects::GetCategoryFromID(o->GetObjectDescription().ID);
#ifdef ENGINE_CSHARP
		if (o->GetObjectDescription().ID == CSharpObject::GetID() && static_cast<CSharpObject*>(o)->CS_Obj.ID != 0)
		{
			auto Classes = CSharpInterop::CSharpSystem->GetAllClasses();
			for (auto& i : Classes)
			{
				size_t LastSlash = i.find_last_of("/");
				std::string Path;
				std::string Name = i;
				if (LastSlash != std::string::npos)
				{
					Path = i.substr(0, LastSlash);
					Name = i.substr(LastSlash + 1);
				}
				if (Name != static_cast<CSharpObject*>(o)->CSharpClass)
				{
					continue;
				}
				CurrentPath = Path;
			}
		}
#endif

		std::vector<std::string> PathElements;
		size_t Index = CurrentPath.find_first_of("/");
		while (Index != std::string::npos)
		{
			Index = CurrentPath.find_first_of("/");
			PathElements.push_back(CurrentPath.substr(0, Index));
			CurrentPath = CurrentPath.substr(Index + 1);
			Index = CurrentPath.find_first_of("/");
		}
		if (!CurrentPath.empty())
		{
			PathElements.push_back(CurrentPath);
		}

		ObjectListItem* CurrentList = SceneList;
		if (SceneList->IsCollapsed) continue;
		for (const auto& elem : PathElements)
		{
			ObjectListItem* NewList = nullptr;
			for (auto& c : CurrentList->Children)
			{
				if (c.Name != elem) continue;
				NewList = &c;
				break;
			}

			if (!NewList && !CurrentList->IsCollapsed)
			{
				int it = 0;
				while (true)
				{
					if (it >= (int)CurrentList->Children.size() || CurrentList->Children[it].Object)
					{
						break;
					}
					it++;
				}
				ObjectCategories.push_back(elem);
				CurrentList->Children.insert(CurrentList->Children.begin() + it,
					ObjectListItem(elem, {}, false, CollapsedItems.contains("OBJ_CAT_" + elem)));
				CurrentList->Children[it].ListIndex = (int)ObjectCategories.size() - 1;
				NewList = &CurrentList->Children[it];
			}
			CurrentList = NewList;
		}
		if (CurrentList && !CurrentList->IsCollapsed)
		{
			CurrentList->Children.push_back(ObjectListItem(o, (int)ListIndex));
		}
		else if (CurrentList && o->IsSelected)
		{
			CurrentList->IsSelected = true;
		}
		ListIndex++;
	}
	return ObjectList;
}
#endif