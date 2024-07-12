#if EDITOR
#include "ClassCreator.h"
#include <UI/EditorUI/EditorUI.h>
#include <filesystem>
#include <Engine/OS.h>
#include <Engine/Application.h>
#include <fstream>
#include <thread>
#include <Engine/Subsystem/BackgroundTask.h>
#include <Engine/Log.h>
#include <UI/EditorUI/ClassesBrowser.h>

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

ClassCreator::ClassCreator() : EditorPopup(0, 0.3f, "Create class")
{
	PopupBackground->SetVerticalAlign(UIBox::Align::Default);

	SetOptions({
		PopupOption("Create"),
		PopupOption("Cancel")
		});

	ClassFields[0] = new UITextField(0, EditorUI::UIColors[1], this, 2, EditorUI::Text);
	ClassFields[1] = new UITextField(0, EditorUI::UIColors[1], this, 2, EditorUI::Text);

	PathText = new UIText(0.4f, EditorUI::UIColors[2], "File: ", EditorUI::Text);

	PopupBackground->AddChild(PathText
		->SetPadding(0.02f));

	PopupBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->AddChild((new UIText(0.4f, EditorUI::UIColors[2], "Path: ", EditorUI::Text))
			->SetTextWidthOverride(0.06f)
			->SetPadding(0.01f, 0.01f, 0.02f, 0))
		->AddChild(ClassFields[1]
			->SetText(ClassesBrowser::GetCurrentCPPPathString().substr(std::string("Classes/").length()))
			->SetTextSize(0.4f)
			->SetPadding(0.01f, 0.01f, 0, 0)
			->SetMinSize(Vector2(0.15f, 0.01f))));

	PopupBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->AddChild((new UIText(0.4f, EditorUI::UIColors[2], "Name: ", EditorUI::Text))
			->SetTextWidthOverride(0.06f)
			->SetPadding(0.01f, 0.01f, 0.02f, 0))
		->AddChild(ClassFields[0]
			->SetText("MyClass")
			->SetTextSize(0.4f)
			->SetPadding(0.01f, 0.01f, 0, 0)
			->SetMinSize(Vector2(0.15f, 0.01f))));

}

void ClassCreator::Tick()
{
	TickPopup();
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

	PathText->SetText("File: Scripts/" + DisplayedPath + ClassFields[0]->GetText() + ".cs");
}

void ClassCreator::OnButtonClicked(int Index)
{
	switch (Index)
	{
	case 0:
	{
		Create(ClassFields[0]->GetText(), PathString, ClassType::CSharp);
		delete this;
		return;
	}
	case 1:
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
		new BackgroundTask(EditorUI::RebuildAssembly);
		break;
	}
#endif
	default:
		break;
	}
}
#endif