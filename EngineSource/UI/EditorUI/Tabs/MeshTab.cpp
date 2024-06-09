#if EDITOR
#include "MeshTab.h"
#include <fstream>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Importers/ModelConverter.h>
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Framebuffer.h>
#include <UI/UIButton.h>
#include <UI/EditorUI/EditorUI.h>
#include <Engine/Application.h>
#include <Engine/File/Assets.h>

namespace MaterialTemplates
{
	struct TemplateParam
	{
		std::string UniformName;
		int NativeType;
		std::string Value;
		TemplateParam(std::string UniformName, int NativeType, std::string Value)
		{
			this->UniformName = UniformName;
			this->NativeType = NativeType;
			this->Value = Value;
		}
	};
}
MeshTab::MeshTab(EditorPanel* Parent, std::string File) : EditorTab(Parent, "Model", File)
{
	if (!PreviewBuffer)
	{
		PreviewBuffer = new FramebufferObject();
		PreviewBuffer->UseMainWindowResolution = true;
		PreviewCamera = new Camera(2, 1600, 900);
		PreviewCamera->Position = Vector3(7, 3, 7);
		PreviewCamera->SetRotation(Vector3(-15, -135, 0));
		PreviewBuffer->FramebufferCamera = PreviewCamera;
	}
	auto RowBox = new UIBox(UIBox::Orientation::Horizontal, 0);
	RowBox->SetVerticalAlign(UIBox::Align::Default);
	PanelMainBackground->AddChild(RowBox
		->SetPadding(0));

	PreviewWindow = new UIBackground(UIBox::Orientation::Horizontal, 0, 1, 0.5f);

	Rows[0] = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	RowBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 0.75))
		->SetPadding(0)
		->AddChild(Rows[0]
			->SetPadding(0)));
	Rows[0]->AddChild(PreviewWindow);
	PreviewWindow->SetBorder(UIBox::BorderType::Rounded, 1);

	Rows[1] = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	RowBox->AddChild((new UIBox(UIBox::Orientation::Vertical, 0))
		->SetPadding(0)
		->AddChild(Rows[1]
			->SetPadding(0)));
}

void MeshTab::Tick()
{
	PreviewBuffer->FramebufferCamera = PreviewCamera;
	PreviewWindow->SetUseTexture(true, PreviewBuffer->GetTextureID());
	Transform PreviousCameraTransform;
	if (CameraTransform != PreviousCameraTransform && PanelMainBackground->IsVisible)
	{
		CameraTransform = PreviousCameraTransform;
	}
	if (RedrawFrames)
	{
		RedrawFrames--;
		if (RedrawFrames == 0)
		{
			PreviewWindow->RedrawElement();
		}
	}
}

void MeshTab::Load(std::string File)
{
	try
	{
		PreviewBuffer->GetBuffer()->ReInit((int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
		ModelData = ModelGenerator::ModelData();
		ModelData.LoadModelFromFile(File);
		HasCollision = ModelData.HasCollision;
		TwoSided = ModelData.TwoSided;
		CastShadow = ModelData.CastShadow;
		InitialName = File;
		UpdatePreviewModel();
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}

	Generate();
}

void MeshTab::ReloadMesh()
{
	try
	{
		ModelData = ModelGenerator::ModelData();
		ModelData.LoadModelFromFile(InitialName);
		HasCollision = ModelData.HasCollision;
		TwoSided = ModelData.TwoSided;
		CastShadow = ModelData.CastShadow;
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}

	Generate();
}

void MeshTab::Save()
{
	try
	{
		ModelData.CastShadow = CastShadow;
		ModelData.HasCollision = HasCollision;
		ModelData.TwoSided = TwoSided;
		ModelData.SaveModelData(InitialName);
		PreviewModel = nullptr;
		PreviewBuffer->ClearContent();
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}
}

void MeshTab::Generate()
{
	Rows[0]->DeleteChildren();
	Rows[1]->DeleteChildren();
	Rows[1]->AddChild(new UIText(0.7f, EditorUI::UIColors[2], "Materials:", EditorUI::Text));
	int Index = -1;
	PreviewWindow = new UIBackground(UIBox::Orientation::Horizontal, 0, 1, 0.3f);
	Rows[0]->AddChild(PreviewWindow
		->SetPadding(0.05f, 0.02f, 0.02f, 0.04f));
	Rows[0]->AddChild(new UIText(0.5f, EditorUI::UIColors[2], "Model properties:", EditorUI::Text));

	for (auto& i : Options)
	{
		auto OptionBox = new UIBox(UIBox::Orientation::Horizontal, 0);
		auto NewText = new UIText(0.5f, EditorUI::UIColors[2], i, EditorUI::Text);
		OptionBox->AddChild(NewText);
		auto NewButton = new UIButton(UIBox::Orientation::Horizontal, 0, 1, this, Index);
		OptionBox->AddChild(NewButton);
		OptionBox->SetPadding(0.01f);
		NewButton
			->SetUseTexture(*OptionVariables[std::abs(Index) - 1], Application::EditorInstance->Textures[16])
			->SetMinSize(0.04f)
			->SetPadding(0, 0, 0.01f, 0)
			->SetSizeMode(UIBox::SizeMode::AspectRelative);
		NewText->SetPadding(0, 0, 0.02f, 0);
		NewText->SetTextWidthOverride(0.15f);
		NewButton->SetBorder(UIBox::BorderType::Rounded, 0.3f);
		Rows[0]->AddChild(OptionBox);
		Index--;
	}
	MaterialTextFields.clear();
	int MaterialIndex = 0;
	for (auto& i : ModelData.Elements)
	{
		auto NewTextInput = new UITextField(0, EditorUI::UIColors[1], this, 1, EditorUI::Text);
		NewTextInput->SetMinSize(Vector2(0.2f, 0.05f));
		NewTextInput->SetTextSize(0.525f);
		NewTextInput->SetTextColor(Assets::GetAsset(i.ElemMaterial + ".jsmat").empty() ? Vector3(1.0, 0.0f, 0.0f) : EditorUI::UIColors[2]);
		NewTextInput->SetText(i.ElemMaterial);
		Rows[1]->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
			->SetPadding(0.01f, 0.0f, 0.03f, 0.0f)
			->SetVerticalAlign(UIBox::Align::Centered)
			->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "Material " + std::to_string(MaterialIndex++) + ":  ", EditorUI::Text))
				->SetPadding(0, 0, 0, 0))
			->AddChild(NewTextInput
				->SetPadding(0)));
		MaterialTextFields.push_back(NewTextInput);
	}
}

void MeshTab::OnButtonClicked(int Index)
{
	HandlePanelButtons(Index);
	switch (Index)
	{
	case 1:
		for (int i = 0; i < ModelData.Elements.size(); i++)
		{
			ModelData.Elements[i].ElemMaterial = MaterialTextFields[i]->GetText();
		}
		Generate();
		UpdatePreviewModel();
		break;
	case -1:
		CastShadow = !CastShadow;
		Generate();
		UpdatePreviewModel();
		break;
	case -2:
		HasCollision = !HasCollision;
		Generate();
		UpdatePreviewModel();
		break;
	case -3:
		TwoSided = !TwoSided;
		Generate();
		UpdatePreviewModel();
		break;
	default:
		break;
	}
}

MeshTab::~MeshTab()
{
	delete PreviewBuffer;
	delete PreviewModel;
}

void MeshTab::OnResized()
{
	Rows[0]->SetMinSize(Vector2(0.35f, Scale.Y));
	Rows[0]->SetMaxSize(Vector2(0.35f, Scale.Y));
	Rows[1]->SetMinSize(Vector2(Scale.X - 0.35f, Scale.Y));
	Rows[1]->SetMaxSize(Vector2(Scale.X - 0.35f, Scale.Y));

}

void MeshTab::UpdatePreviewModel()
{
	Save();
	PreviewModel = new Model(InitialName);
	PreviewBuffer->ClearContent();
	PreviewBuffer->UseWith(PreviewModel);
	PreviewModel->UpdateTransform();
	RedrawFrames = 3;
}
#endif