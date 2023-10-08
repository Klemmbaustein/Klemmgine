#if EDITOR
#include "ClassCreator.h"
#include <UI/EditorUI/EditorUI.h>
#include <filesystem>
#include <Engine/OS.h>

ClassCreator::ClassCreator() : EditorPanel(Editor::CurrentUI->UIColors, 0.25, 1, 0.5, 0.5, true, "Class creator")
{
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5f);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);
	TabBackground->SetAlign(UIBox::Align::Default);
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

	// TODO: Implement actual functionality
	TabBackground->AddChild(new UIText(2, 1, "TODO", Editor::CurrentUI->EngineUIText));

	UpdateLayout();
}

void ClassCreator::UpdateLayout()
{
	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075f));
}

void ClassCreator::Tick()
{
	UpdatePanel();
}

void ClassCreator::OnButtonClicked(int Index)
{
	switch (Index)
	{
	case -2:
		//Create("MyClass", ClassType::CPlusPlus);
		delete this;
		return;
	case -1:
		delete this;
		return;
	}
}
void ClassCreator::Create(std::string Name, ClassType NewType)
{
	switch (NewType)
	{
	case ClassCreator::ClassType::CPlusPlus:
		std::filesystem::copy("../../EditorContent/CodeTemplates/Class.h", "Code/Objects/Class.h");
		std::filesystem::copy("../../EditorContent/CodeTemplates/Class.cpp", "Code/Objects/Class.cpp");
		system("\"..\\..\\Tools\\bin\\BuildTool.exe\" in=../../EngineSource/Objects in=Code/Objects out=GeneratedIncludes");
		system(("cd ../.. && ProjectGenerator.exe -projectName " + Build::GetProjectBuildName() + " -onlyBuildFiles").c_str());
		break;
#if ENGINE_CSHARP
	case ClassCreator::ClassType::CSharp:
		break;
#endif
	default:
		break;
	}
}
#endif