#ifdef EDITOR
#pragma once
#include "MeshTab.h"
#include <fstream>
#include <Engine/Utility/FileUtility.h>
#include <Engine/Importers/ModelConverter.h>
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Utility/Framebuffer.h>
#include <UI/UIButton.h>
#include <UI/EditorUI/EditorUI.h>

#define MESHTAB_DEBUG 1

namespace MaterialTemplates
{
	struct TemplateParam
	{
		std::string UniformName;
		int Type;
		std::string Value;
		TemplateParam(std::string UniformName, int Type, std::string Value)
		{
			this->UniformName = UniformName;
			this->Type = Type;
			this->Value = Value;
		}
	};
}
MeshTab::MeshTab(Vector3* UIColors, TextRenderer* Renderer) : EditorTab(UIColors)
{
	this->Renderer = Renderer;
	TabName = new UIText(1, UIColors[2], "Model: " + FileUtil::GetFileNameWithoutExtensionFromPath(Filepath), Renderer);
	TabName->SetPadding(0.1f, 0.05f, 0.05f, 0);
	TabBackground->AddChild(TabName);
	TabBackground->SetAlign(UIBox::Align::Reverse);
	if (!PreviewBuffer)
	{
		PreviewBuffer = new FramebufferObject();
		PreviewBuffer->UseMainWindowResolution = true;
		Camera* Cam = new Camera(2, 1600, 900);
		Cam->Position = glm::vec3(15, 0, 40);
		Cam->yaw = 90;
		PreviewBuffer->FramebufferCamera = Cam;
	}
	auto RowBox = new UIBox(true, 0);
	TabBackground->AddChild(RowBox);
	Rows[0] = new UIBackground(false, 0, UIColors[1]);
	RowBox->AddChild(Rows[0]);
	PreviewWindow = new UIBackground(true, 0, 1, 0.5f);
	Rows[0]->AddChild(PreviewWindow);
	Rows[0]->SetMinSize(Vector2(0, 1.1f));
	Rows[0]->SetMaxSize(Vector2(1, 1.1f));
	Rows[0]->SetAlign(UIBox::Align::Reverse);
	PreviewWindow->SetBorder(UIBox::BorderType::Rounded, 1);

	Rows[1] = new UIScrollBox(false, 0, true);
	Rows[1]->SetAlign(UIBox::Align::Reverse);
	Rows[1]->SetMinSize(Vector2(1.0f, 1.0f));
	Rows[1]->SetMaxSize(Vector2(1.0f, 1.0f));
	RowBox->AddChild((new UIBox(false, 0))
		->AddChild(Rows[1])
		->AddChild(new UIText(0.7f, UIColors[2], "Materials", Renderer)));
}

void MeshTab::Tick()
{
	try
	{
		PreviewBuffer->FramebufferCamera = Graphics::MainCamera;
		PreviewWindow->SetUseTexture(true, PreviewBuffer->GetTextureID());
		Transform PreviousCameraTransform;
		PreviousCameraTransform.Location = Graphics::MainCamera->Position;
		PreviousCameraTransform.Rotation = Graphics::MainCamera->Rotation;
		if (CameraTransform != PreviousCameraTransform && TabBackground->IsVisible)
		{
			CameraTransform = PreviousCameraTransform;
			UIBox::RedrawUI();
		}
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
		throw(UIRenderingException("MeshTab", e.what()));
	}
}

void MeshTab::Load(std::string File)
{
	try
	{
		TabName->SetText(FileUtil::GetFileNameWithoutExtensionFromPath(File));
		PreviewBuffer->GetBuffer()->ReInit((int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
		Graphics::MainCamera->Position = Vector3(0, 4, 15);
		Graphics::MainCamera->SetRotation(Vector3(0, -90, 0));
		ModelData = ModelGenerator::ModelData();
		ModelData.LoadModelFromFile(File);
		if (!MESHTAB_DEBUG)
		{
			for (auto& m : ModelData.Elements)
			{
				m.ElemMaterial = m.ElemMaterial.substr(8);
			}
		}
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
		ModelData.SaveModelData(InitialName, !MESHTAB_DEBUG);
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
	int Index = -1;
	PreviewWindow = new UIBackground(true, 0, 1, 0.3f);
	Rows[0]->AddChild(PreviewWindow);
	Rows[0]->AddChild(new UIText(0.5f, UIColors[2], "Model properties", Renderer));

	for (auto& i : Options)
	{
		auto OptionBox = new UIBox(true, 0);
		auto NewText = new UIText(0.5f, UIColors[2], i, Renderer);
		OptionBox->AddChild(NewText);
		auto NewButton = new UIButton(true, 0, 1, this, Index);
		OptionBox->AddChild(NewButton);
		OptionBox->SetPadding(0.01f);
		NewButton
			->SetUseTexture(*OptionVariables[std::abs(Index) - 1], Editor::CurrentUI->Textures[16])
			->SetMinSize(0.04f)
			->SetPadding(0, 0, 0.01f, 0)
			->SetSizeMode(UIBox::SizeMode::PixelRelative);
		NewText->SetPadding(0, 0, 0.02f, 0);
		NewButton->SetBorder(UIBox::BorderType::Rounded, 0.3f);
		Rows[0]->AddChild(OptionBox);
		Index--;
	}
	MaterialTextFields.clear();
	int MaterialIndex = 0;
	for (auto& i : ModelData.Elements)
	{
		auto NewTextInput = new UITextField(true, 0, UIColors[1], this, 1, Renderer);
		NewTextInput->SetMinSize(Vector2(0.4f, 0.05f));
		NewTextInput->SetText(i.ElemMaterial);
		Rows[1]->AddChild((new UIBox(true, 0))
			->SetPadding(0)
			->AddChild((new UIText(0.5f, UIColors[2], "Material " + std::to_string(MaterialIndex++) + ": ", Renderer))
				->SetPadding(0, 0.03f, 0, 0))
			->AddChild(NewTextInput));
		MaterialTextFields.push_back(NewTextInput);
	}
	UIBox::DrawAllUIElements();
	UIBox::RedrawUI();
	Rows[1]->SetMinSize(Vector2(1.0f, 1.0f));
	Rows[1]->SetMaxSize(Vector2(1.0f, 1.0f));
}

void MeshTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible) return;
	switch (Index)
	{
	case 1:
		for (int i = 0; i < ModelData.Elements.size(); i++)
		{
			ModelData.Elements[i].ElemMaterial = MaterialTextFields[i]->GetText();
		}
		Generate();
		break;
	case -1:
		CastShadow = !CastShadow;
		Generate();
		break;
	case -2:
		HasCollision = !HasCollision;
		Generate();
		break;
	case -3:
		TwoSided = !TwoSided;
		Generate();
		break;
	default:
		break;
	}
	UpdatePreviewModel();
}

MeshTab::~MeshTab()
{
}

void MeshTab::UpdatePreviewModel()
{
	Save();
	PreviewModel = new Model(InitialName);
	PreviewBuffer->ClearContent();
	PreviewBuffer->UseWith(PreviewModel);
	PreviewModel->UpdateTransform();
}
#endif