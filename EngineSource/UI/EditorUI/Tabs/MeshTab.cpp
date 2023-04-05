#ifdef EDITOR
#pragma once
#include "MeshTab.h"
#include <fstream>
#include <Engine/FileUtility.h>
#include <Engine/Importers/ModelConverter.h>
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Utility/Framebuffer.h>
#include <UI/UIButton.h>

#define MESHTAB_DEBUG 0

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

	TabBackground->Align = UIBox::E_REVERSE;
	TabName = new UIText(1, 1, "Model: ", Renderer);
	TabName->SetPadding(0.1, 0.05, 0.05, 0);
	if (!PreviewBuffer)
	{
		PreviewBuffer = new FramebufferObject();
		PreviewBuffer->UseMainWindowResolution = true;
		Camera* Cam = new Camera(2, 1600, 900);
		Cam->Position = glm::vec3(15, 0, 40);
		Cam->yaw = 90;
		PreviewBuffer->FramebufferCamera = Cam;
	}
	TabBackground->AddChild(TabName);
	auto RowBox = new UIBox(true, 0);
	TabBackground->AddChild(RowBox);
	Rows[0] = new UIScrollBox(false, 0, 20);
	RowBox->AddChild(Rows[0]);
	PreviewWindow = new UIBackground(true, 0, 1, 0.5);
	Rows[0]->AddChild(PreviewWindow);
	PreviewWindow->SetBorder(UIBox::E_ROUNDED, 1);

	Rows[1] = new UIScrollBox(false, 0, 20);
	Rows[1]->Align = UIBox::E_REVERSE;
	RowBox->AddChild(Rows[1]);
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
		PreviewBuffer->GetBuffer()->ReInit(Graphics::WindowResolution.X, Graphics::WindowResolution.Y);
		Graphics::MainCamera->Position = Vector3(0, 4, 15);
		Graphics::MainCamera->SetRotation(Vector3(0, -90, 0));
		TabName->SetText("Model: " + FileUtil::GetFileNameWithoutExtensionFromPath(File));
		ModelData = ModelGenerator::ModelData();
		ModelData.LoadModelFromFile(File);
		if (!MESHTAB_DEBUG)
		{
			for (std::string& m : ModelData.Materials)
			{
				m = m.substr(8);
			}
		}
		NumTotalVertices = ModelData.GetMergedVertices().size();
		HasCollision = ModelData.HasCollision;
		TwoSided = ModelData.TwoSided;
		CastShadow = ModelData.CastShadow;
		Materials = ModelData.Materials;
		MeshVertices = ModelData.Vertices;
		Indices = ModelData.Indices;
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
		NumTotalVertices = ModelData.GetMergedVertices().size();
		HasCollision = ModelData.HasCollision;
		TwoSided = ModelData.TwoSided;
		CastShadow = ModelData.CastShadow;
		Materials = ModelData.Materials;
		MeshVertices = ModelData.Vertices;
		Indices = ModelData.Indices;
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
		ModelData.Materials = Materials;
		ModelData.CastShadow = CastShadow;
		ModelData.HasCollision = HasCollision;
		ModelData.TwoSided = TwoSided;
		ModelData.SaveModelData(InitialName, !MESHTAB_DEBUG);
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
	for (auto& i : Options)
	{
		auto OptionBox = new UIBox(true, 0);
		auto NewText = new UIText(0.7, 1, i, Renderer);
		OptionBox->AddChild(NewText);
		auto NewButton = new UIButton(true, 0, UIColors[0] * 2, this, Index);
		OptionBox->AddChild(NewButton);
		OptionBox->SetPadding(0.01);
		NewButton->SetPadding(0, 0, 0.01, 0);
		NewText->SetPadding(0);
		auto ButtonText = new UIText(0.7, 1, *OptionVariables[std::abs(Index) - 1] ? "true" : "false", Renderer);
		NewButton->SetBorder(UIBox::E_ROUNDED, 1);
		ButtonText->SetPadding(0.01);
		NewButton->SetMinSize(Vector2(0.1, 0.075));
		NewButton->AddChild(ButtonText);
		Rows[0]->AddChild(OptionBox);
		Index--;
	}
	auto Text = new UIText(0.7, 1, "Materials", Renderer);
	Rows[1]->AddChild(Text);
	MaterialTextFields.clear();
	for (auto& i : Materials)
	{
		auto NewTextInput = new UITextField(true, 0, UIColors[1], this, 1, Renderer);
		NewTextInput->SetMinSize(Vector2(0.4, 0.075));
		NewTextInput->SetText(i);
		NewTextInput->SetBorder(UIBox::E_ROUNDED, 0.5);
		Rows[1]->AddChild(NewTextInput);
		MaterialTextFields.push_back(NewTextInput);
	}
	PreviewWindow = new UIBackground(true, 0, 1, 0.5);
	Rows[0]->AddChild(PreviewWindow);
	PreviewWindow->SetBorder(UIBox::E_ROUNDED, 1);
}

void MeshTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible) return;
	switch (Index)
	{
	case 1:
		for (int i = 0; i < Materials.size(); i++)
		{
			Materials[i] = MaterialTextFields[i]->GetText();
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