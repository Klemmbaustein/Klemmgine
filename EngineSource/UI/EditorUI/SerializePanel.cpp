#if EDITOR
#include "SerializePanel.h"
#include <UI/EditorUI/LogUI.h>
#include <UI/EditorUI/Toolbar.h>
#include <UI/EditorUI/AssetBrowser.h>
#include <UI/EditorUI/ClassesBrowser.h>
#include <UI/EditorUI/StatusBar.h>
#include <UI/EditorUI/Viewport.h>
#include <UI/EditorUI/ObjectList.h>
#include <UI/EditorUI/ContextMenu.h>
#include <UI/EditorUI/SettingsPanel.h>
#include <Engine/Application.h>
#include <Engine/Log.h>

std::string Editor::SerializePanel::GetLayoutPrefFilePath()
{
	return Application::GetEditorPath() + "/EditorContent/Config/EditorLayout";
}

SaveData::Field Editor::SerializePanel::SerializeLayout(EditorPanel* Target)
{
	SaveData::Field f;
	f.Name = "root";
	f.Type = NativeType::Object;

	std::string AlignString;
	switch (Target->ChildrenAlign)
	{
	case EditorPanel::ChildrenType::Tabs:
		AlignString = "tabs";
		break;
	case EditorPanel::ChildrenType::Horizontal:
		AlignString = "horizontal";
		break;
	case EditorPanel::ChildrenType::Vertical:
	default:
		AlignString = "vertical";
		break;
	}

	f.Children =
	{
		SaveData::Field(NativeType::Float, "size", std::to_string(Target->Size)),
		SaveData::Field(NativeType::Bool, "collapsed", std::to_string(Target->Collapsed)),
		SaveData::Field(NativeType::String, "align", AlignString),
		SaveData::Field(NativeType::String, "type", Target->ClassName),
	};

	SaveData::Field ChildrenObject;
	ChildrenObject.Type = NativeType::Object;
	ChildrenObject.Name = "children";
	size_t it = 0;
	for (EditorPanel* i : Target->Children)
	{
		SaveData::Field f = SerializeLayout(i);
		f.Name = i->ClassName + "_" + std::to_string(it++);
		ChildrenObject.Children.push_back(f);
	}

	if (ChildrenObject.Children.size())
	{
		f.Children.push_back(ChildrenObject);
	}

	return f;
}

EditorPanel* Editor::SerializePanel::DeSerializeLayout(SaveData::Field From, EditorPanel* Parent)
{
	Stats::EngineStatus = "Loading editor layout";
	EditorPanel* New = nullptr;
	std::string ClassName = From.At("type").AsString();
	if (!Parent)
	{
		New = new EditorPanel(-1, Vector2(2, 1.95f), "root", "root");
	}
	else
	{
		if (ClassName == "viewport")
		{
			New = new Viewport(Parent);
		}
		else if (ClassName == "object_list")
		{
			New = new ObjectList(Parent);
		}
		else if (ClassName == "file_browser")
		{
			New = new AssetBrowser(Parent);
		}
		else if (ClassName == "class_browser")
		{
			New = new ClassesBrowser(Parent);
		}
		else if (ClassName == "log")
		{
			New = new LogUI(Parent);
		}
		else if (ClassName == "toolbar")
		{
			New = new Toolbar(Parent);
		}
		else if (ClassName == "context_menu_scene")
		{
			New = new ContextMenu(Parent, true);
		}
		else if (ClassName == "context_menu_obj")
		{
			New = new ContextMenu(Parent, false);
		}
		else if (ClassName == "settings")
		{
			New = new SettingsPanel(Parent);
		}
		else
		{
			New = new EditorPanel(Parent, "panel");
		}
	}

	New->Size = From.At("size").AsFloat();
	New->Collapsed = From.At("collapsed").AsBool();

	std::string AlignString = From.At("align").AsString();
	if (AlignString == "tabs")
	{
		New->ChildrenAlign = EditorPanel::ChildrenType::Tabs;
	}
	else if (AlignString == "horizontal")
	{
		New->ChildrenAlign = EditorPanel::ChildrenType::Horizontal;
	}
	else
	{
		New->ChildrenAlign = EditorPanel::ChildrenType::Vertical;
	}

	if (From.Contains("children"))
	{
		for (auto& i : From.At("children").Children)
		{
			DeSerializeLayout(i, New);
		}
	}
	New->UpdatePanelLayout = true;

	return New;
}
#endif