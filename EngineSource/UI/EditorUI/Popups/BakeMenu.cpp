#if EDITOR
#include "BakeMenu.h"
#include <UI/EditorUI/EditorUI.h>
#include <Rendering/RenderSubsystem/BakedLighting.h>
#include <Engine/Utility/FileUtility.h>
#include <filesystem>
#include <UI/UIScrollBox.h>
#include <Engine/Input.h>
#include <Engine/Log.h>
#include <Engine/Application.h>

BakeMenu* BakeMenu::ActiveBakeMenu = nullptr;

BakeMenu::BakeMenu()
	: EditorPopup(0, Vector2(0.5f, 0.55f), "Bake scene lighting")
{
	if (ActiveBakeMenu)
	{
		delete this;
		return;
	}

	ActiveBakeMenu = this;

	InputFields[0] = new UITextField(0, EditorUI::UIColors[1], this, 2, EditorUI::Text);
	InputFields[1] = new UITextField(0, EditorUI::UIColors[1], this, 2, EditorUI::Text);

	PopupBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->SetPadding(0.01f, 0.01f, 0.01f, 0.01f)
		->AddChild((new UIText(0.55f, EditorUI::UIColors[2], "Lightmap scale:", EditorUI::Text))
			->SetTextWidthOverride(0.2f)
			->SetPadding(0.005f))
		->AddChild(InputFields[0]
			->SetText(EditorUI::ToShortString(BakedLighting::LightmapScaleMultiplier))
			->SetTextSize(0.5f)
			->SetPadding(0, 0, 0, 0)
			->SetMinSize(Vector2(0.1f, 0.01f))));

	PopupBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->SetPadding(0.01f)
		->AddChild((new UIText(0.55f, EditorUI::UIColors[2], "Lightmap resolution:", EditorUI::Text))
			->SetTextWidthOverride(0.2f)
			->SetPadding(0.005f))
		->AddChild(InputFields[1]
			->SetText(std::to_string(BakedLighting::LightmapResolution))
			->SetTextSize(0.5f)
			->SetPadding(0, 0, 0, 0)
			->SetMinSize(Vector2(0.1f, 0.01f))));

	SetOptions({
	PopupOption("Bake"),
	PopupOption("Cancel")
		});
}

BakeMenu::~BakeMenu()
{
	if (ActiveBakeMenu == this)
	{
		ActiveBakeMenu = nullptr;
	}
}

void BakeMenu::OnButtonClicked(int Index)
{
	if (IsFinished)
	{
		delete this;
		return;
	}
	if (Index == 1)
	{
		delete this;
		return;
	}
	if (Index == 0)
	{
		BakedLighting::LightmapScaleMultiplier = std::stof(InputFields[0]->GetText());
		BakedLighting::LightmapResolution = std::stoull(InputFields[1]->GetText());
		Application::EditorInstance->BakeScene();
		StartBake();
		return;
	}
}

void BakeMenu::Tick()
{
	TickPopup();

	if (BakeProgressText)
	{
		if (!IsFinished && !EditorUI::IsBakingScene)
		{
			SetOptions({EditorPopup::PopupOption("Close")});
			IsFinished = true;
		}
		else if (EditorUI::IsBakingScene)
		{
			BakeProgressText->SetText("Progress: " + std::to_string((int)(BakedLighting::GetBakeProgress() * 100)) + "%");
		}
		else
		{
			BakeProgressText->SetText("Progress: 100%");
		}
	}

	if (BakedLighting::GetBakeLog().size() != CurrentBakeProgress && LogScrollBox)
	{
		CurrentBakeProgress = BakedLighting::GetBakeLog().size();
		GenerateBakeLog();
	}
}

void BakeMenu::GenerateBakeLog()
{
	LogScrollBox->DeleteChildren();
	for (auto& i : BakedLighting::GetBakeLog())
	{
		LogScrollBox->AddChild((new UIText(0.45f, EditorUI::UIColors[2], i, EditorUI::MonoText))
			->SetPadding(0, 0, 0.01f, 0));
	}
}

void BakeMenu::StartBake()
{
	PopupBackground->DeleteChildren();
	SetOptions({});
	
	PopupBackground->AddChild((new UIText(0.6f, EditorUI::UIColors[2], "Baking lighting...", EditorUI::Text))
		->SetPadding(0.01f, 0.01f, 0.02f, 0.005f));

	BakeProgressText = new UIText(0.5, EditorUI::UIColors[2], "Progress: 0%", EditorUI::Text);
	PopupBackground->AddChild(BakeProgressText
		->SetPadding(0.01f, 0.01f, 0.02f, 0.005f));

	LogScrollBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	LogScrollBox->SetMinSize(Vector2(0.45f, 0.35f));
	LogScrollBox->SetMaxSize(Vector2(0.45f, 0.35f));
	LogScrollBox->SetPadding(0);

	PopupBackground->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[1], 0))
		->SetPadding(0.01f, 0.01f, 0.02f, 0.01f)
		->AddChild(LogScrollBox));
}
#endif