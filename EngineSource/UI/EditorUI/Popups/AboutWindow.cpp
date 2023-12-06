#if EDITOR
#include "AboutWindow.h"
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Log.h>
#include <Engine/EngineProperties.h>
#include <CSharp/CSharpInterop.h>

AboutWindow::AboutWindow()
	: EditorPanel(Editor::CurrentUI->UIColors, Position, Vector2(0.4f, 0.4f), Vector2(0.4f, 0.4f), Vector2(0.4f, 0.4f), true, "About")
{
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetVerticalAlign(UIBox::Align::Centered);
	ButtonBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);

	ButtonBackground->AddChild(
		(new UIButton(true, 0, UIColors[2], this, (int)0))
		->SetPadding(0.01f)
		->SetBorder(UIBox::BorderType::Rounded, 0.2f)
		->AddChild((new UIText(0.45f, 1 - UIColors[2], "Ok", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005f)));

	TabBackground->SetVerticalAlign(UIBox::Align::Default);
	TabBackground->AddChild(ButtonBackground);
	ContentBox = new UIBox(false, 0);
	ContentBox
		->SetPadding(0)
		->SetMinSize(0.3f);
	TabBackground->AddChild(ContentBox);

	ContentBox->AddChild((new UIBox(true, 0))
		->SetPadding(0)
		->AddChild((new UIBackground(true, 0, 1, 0.1f))
			->SetUseTexture(true, Editor::CurrentUI->Textures[15])
			->SetPadding(0.01f)
			->SetSizeMode(UIBox::SizeMode::PixelRelative))
		->AddChild((new UIBox(false, 0))
			->SetPadding(0)
			->AddChild((new UIText(0.5f, UIColors[2], "Klemmgine Editor v" + std::string(VERSION_STRING), Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f))
#if ENGINE_CSHARP
			->AddChild((new UIText(0.4f, UIColors[2], std::string("  C#: ") + (CSharp::GetUseCSharp() ? "Yes" : "Disabled"), Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f))
#else
			->AddChild((new UIText(0.4f, UIColors[2], "  C#: No", Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f))
#endif
#if ENGINE_NO_SOURCE
			->AddChild((new UIText(0.4f, UIColors[2], "  With pre-built binaries", Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f))
			->AddChild((new UIText(0.4f, UIColors[2], "  Build date: " 
				+ std::string(__DATE__)
				+ " - "
				+ std::string(__TIME__), Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f))
#endif
	));

	// TODO: Credits
	// ContentBox->AddChild(new UIText(0.5, 1, "TODO: Credits", Editor::CurrentUI->EngineUIText));

	UpdateLayout();
}

void AboutWindow::UpdateLayout()
{

	ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075f));
}

AboutWindow::~AboutWindow()
{
}

void AboutWindow::OnButtonClicked(int Index)
{
	delete this;
	return;
}

void AboutWindow::Tick()
{
	UpdatePanel();
}
#endif