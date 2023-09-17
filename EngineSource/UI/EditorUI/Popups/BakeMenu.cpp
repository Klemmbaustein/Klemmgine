#if EDITOR
#include "BakeMenu.h"
#include <UI/EditorUI/EditorUI.h>
#include <Rendering/Utility/BakedLighting.h>
#include <Engine/Utility/FileUtility.h>
#include <filesystem>
#include <UI/UIScrollBox.h>
#include <Engine/Input.h>
#include <Engine/Log.h>


bool BakeMenu::BakeMenuActive = false;

BakeMenu::BakeMenu()
	: EditorPanel(Editor::CurrentUI->UIColors, Position, Vector2(0.5f, 0.55f), Vector2(0.5f, 0.55f), 2, true, "Bake Lightmap")
{
	if (BakeMenuActive)
	{
		return;
	}

	BakeMenuActive = true;
	ButtonBackground = new UIBackground(true, 0, UIColors[0] * 1.5f);
	ButtonBackground->SetPadding(0);
	ButtonBackground->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f);
	TabBackground->SetAlign(UIBox::Align::Default);
	TabBackground->AddChild(ButtonBackground);

	ButtonBackground->AddChild((new UIButton(true, 0, UIColors[2], this, -2))
		->SetPadding(0.01f)
		->SetBorder(UIBox::BorderType::Rounded, 0.2f)
		->AddChild((new UIText(0.45f, 1 - UIColors[2], "Bake", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005f)))
		->AddChild((new UIButton(true, 0, UIColors[2], this, -1))
			->SetPadding(0.01f)
			->SetBorder(UIBox::BorderType::Rounded, 0.2f)
			->AddChild((new UIText(0.45f, 1 - UIColors[2], "Cancel", Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005f)));

	InputFields[0] = new UITextField(true, 0, UIColors[1], this, 0, Editor::CurrentUI->EngineUIText);
	InputFields[1] = new UITextField(true, 0, UIColors[1], this, 0, Editor::CurrentUI->EngineUIText);



	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0.01f, 0.3f, 0.01f, 0.01f)
		->AddChild((new UIText(0.55f, UIColors[2], "Lightmap scale:      ", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005f))
		->AddChild(InputFields[0]
			->SetText(EditorUI::ToShortString(BakedLighting::LightmapScaleMultiplier))
			->SetTextSize(0.5f)
			->SetPadding(0, 0, 0, 0)
			->SetMinSize(Vector2(0.1f, 0.01f))));

	TabBackground->AddChild((new UIBox(true, 0))
		->SetPadding(0.01f)
		->AddChild((new UIText(0.55f, UIColors[2], "Lightmap resolution: ", Editor::CurrentUI->EngineUIText))
			->SetPadding(0.005f))
		->AddChild(InputFields[1]
			->SetText(std::to_string(BakedLighting::LightmapResolution))
			->SetTextSize(0.5f)
			->SetPadding(0, 0, 0, 0)
			->SetMinSize(Vector2(0.1f, 0.01f))));

	UpdateLayout();
}

void BakeMenu::UpdateLayout()
{
	if (ButtonBackground)
	{
		ButtonBackground->SetMinSize(Vector2(TabBackground->GetMinSize().X, 0.075f));
	}
}

BakeMenu::~BakeMenu()
{
	BakeMenuActive = false;
}

void BakeMenu::OnButtonClicked(int Index)
{
	if (Index == -1)
	{
		delete this;
		return;
	}
	if (Index == -2)
	{
		BakedLighting::LightmapScaleMultiplier = std::stof(InputFields[0]->GetText());
		BakedLighting::LightmapResolution = std::stoull(InputFields[1]->GetText());
		Editor::CurrentUI->BakeScene();
		StartBake();
		return;
	}
}

void BakeMenu::Tick()
{
	UpdatePanel();

	if (BakeProgressText)
	{

		if (!IsFinished && !Editor::IsBakingScene)
		{
			IsFinished = true;
			TabBackground->AddChild((new UIButton(true, 0, UIColors[2], this, -1))
				->SetPadding(0.01f)
				->SetBorder(UIBox::BorderType::Rounded, 0.2f)
				->AddChild((new UIText(0.45f, 1 - UIColors[2], "Close", Editor::CurrentUI->EngineUIText))
					->SetPadding(0.005f)));
		}
		else if (Editor::IsBakingScene)
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
		LogScrollBox->AddChild((new UIText(0.45f, UIColors[2], i, Editor::CurrentUI->EngineUIText))
			->SetPadding(0, 0, 0.01f, 0));
	}
}

void BakeMenu::StartBake()
{
	TabBackground->DeleteChildren();
	ButtonBackground = nullptr;
	
	TabBackground->SetAlign(UIBox::Align::Reverse);

	TabBackground->AddChild((new UIText(0.6f, UIColors[2], "Baking lighting...", Editor::CurrentUI->EngineUIText))
		->SetPadding(0.01f, 0.01f, 0.02f, 0.005f));

	BakeProgressText = new UIText(0.5, UIColors[2], "Progress: 0%", Editor::CurrentUI->EngineUIText);
	TabBackground->AddChild(BakeProgressText
		->SetPadding(0.01f, 0.01f, 0.02f, 0.005f));

	LogScrollBox = new UIScrollBox(false, 0, true);
	LogScrollBox->SetMinSize(Vector2(0.45f, 0.35f));
	LogScrollBox->SetMaxSize(Vector2(0.45f, 0.35f));
	LogScrollBox->SetAlign(UIBox::Align::Reverse);
	LogScrollBox->SetPadding(0);

	TabBackground->AddChild((new UIBackground(true, 0, UIColors[1], 0))
		->SetPadding(0.01f, 0.01f, 0.02f, 0.01f)
		->AddChild(LogScrollBox));
}
#endif