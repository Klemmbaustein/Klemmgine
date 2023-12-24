#if EDITOR
#include "ClassCreator.h"
#include <UI/EditorUI/EditorUI.h>
#include <filesystem>
#include <Engine/OS.h>
#include <Engine/Application.h>
#include <fstream>
#include <thread>
#include <Engine/BackgroundTask.h>
#include <Engine/Log.h>
#include <UI/EditorUI/ItemBrowser.h>

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

ClassCreator::ClassCreator() : EditorPanel(Editor::CurrentUI->UIColors, 0.15f, 1, 0.3f, 0.3f, true, "Class creator")
{
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5f);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetVerticalAlign(UIBox::Align::Centered);
	ButtonBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);
	TabBackground->SetVerticalAlign(UIBox::Align::Default);
	TabBackground->AddChild(ButtonBackground);

	ButtonBackground->AddChild((new UIButton(true, 0, UIColors[2], this, -2))
		->SetPadding(0.01f)
		->SetBorder(UIBox::BorderType::Rounded, 0.2f)
		->AddChild((new UIText(0.45f, 1 - UIColors[2], "Create", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005f)))
		->AddChild((new UIButton(true, 0, UIColors[2], this, -1))
			->SetPadding(0.01f)
			->SetBorder(UIBox::BorderType::Rounded, 0.2f)
			->AddChild((new UIText(0.45f, 1 - UIColors[2], "Cancel", Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f)));

	ClassFields[0] = new UITextField(0, UIColors[1], this, 0, Editor::CurrentUI->EngineUIText);
	ClassFields[1] = new UITextField(0, UIColors[1], this, 0, Editor::CurrentUI->EngineUIText);

	PathText = new UIText(0.4f, UIColors[2], "File: ", Editor::CurrentUI->EngineUIText);

	TabBackground->AddChild(PathText);

	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0)
		->AddChild((new UIText(0.4f, UIColors[2], "Path: ", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01f, 0.01f, 0.02f, 0))
		->AddChild(ClassFields[1]
			->SetText(ItemBrowser::GetCurrentCPPPathString().substr(std::string("Classes/").length()))
			->SetTextSize(0.4f)
			->SetPadding(0.01f, 0.01f, 0, 0)
			->SetMinSize(Vector2(0.2f, 0.01f))));

	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0)
		->AddChild((new UIText(0.4f, UIColors[2], "Name: ", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.01f, 0.01f, 0.02f, 0))
		->AddChild(ClassFields[0]
			->SetText("MyClass")
			->SetTextSize(0.4f)
			->SetPadding(0.01f, 0.01f, 0, 0)
			->SetMinSize(Vector2(0.2f, 0.01f))));


	UpdateLayout();
}

void ClassCreator::UpdateLayout()
{
	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075f));
}

void ClassCreator::Tick()
{
	UpdatePanel();

	PathString = ClassFields[1]->GetText();
	if (!PathString.empty() && PathString[PathString.size() - 1] == '/')
	{
		PathString.pop_back();
	}

	std::string DisplayedPath = PathString;
	if (!DisplayedPath.empty())
	{
		DisplayedPath.append("/");
	}

	PathText->SetText("File: " + DisplayedPath + ClassFields[0]->GetText() + ".cs");
}

void ClassCreator::OnButtonClicked(int Index)
{
	switch (Index)
	{
	case -2:
	{
		Create(ClassFields[0]->GetText(), PathString, ClassType::CSharp);
		delete this;
		return;
	}
	case -1:
		delete this;
		return;
	}
}
void ClassCreator::Create(std::string Name, std::string Namespace, ClassType NewType)
{
	switch (NewType)
	{
#if _WIN32
	case ClassCreator::ClassType::CPlusPlus:
		std::filesystem::copy(Application::GetEditorPath() + "/EditorContent/CodeTemplates/Class.h", "Code/Objects/Class.h");
		std::filesystem::copy(Application::GetEditorPath() + "/EditorContent/CodeTemplates/Class.cpp", "Code/Objects/Class.cpp");
		system("\"..\\..\\Tools\\bin\\Bui.ldToolexe\" in=../../EngineSource/Objects in=Code/Objects out=GeneratedIncludes");
		system(("cd ../.. && ProjectGenerator.exe -projectName " + Build::GetProjectBuildName() + " -onlyBuildFiles").c_str());
		break;
#endif
#if ENGINE_CSHARP
	case ClassCreator::ClassType::CSharp:
	{
		std::string TemplateName;
		if (Namespace.empty())
		{
			TemplateName = Application::GetEditorPath() + "/EditorContent/CodeTemplates/Class.cs";
		}
		else
		{
			TemplateName = Application::GetEditorPath() + "/EditorContent/CodeTemplates/ClassNamespace.cs";
		}
		std::ifstream Template = std::ifstream(TemplateName);
		std::stringstream TemplateStream;
		TemplateStream << Template.rdbuf();
		std::string TemplateString = TemplateStream.str();
		Template.close();

		std::string DisplayedPath = Namespace;
		if (!DisplayedPath.empty())
		{
			DisplayedPath.append("/");
		}

		Log::Print("Creating class " + Name + " in Scripts/" + DisplayedPath + Name + ".cs", Log::LogColor::Blue);
		replace(TemplateString, "$", Name);
		std::filesystem::create_directories("Scripts/" + Namespace);
		std::ofstream out = std::ofstream("Scripts/" + Namespace + "/" + Name + ".cs");
		replace(Namespace, "/", ".");
		replace(TemplateString, "@", Namespace);
		out << TemplateString;
		out.close();
		new BackgroundTask(EditorUI::RebuildAndHotReload);
		break;
	}
#endif
	default:
		break;
	}
}
#endif