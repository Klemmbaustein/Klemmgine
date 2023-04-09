#ifdef EDITOR
#pragma once
#include "CubemapTab.h"
#include <fstream>
#include <Engine/FileUtility.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Utility/Framebuffer.h>
#include <UI/UIButton.h>
#include <Rendering/Texture/Cubemap.h>
#include <Engine/Save.h>

CubemapTab::CubemapTab(Vector3* UIColors, TextRenderer* Renderer) : EditorTab(UIColors)
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
	CubemapSidesBox = new UIBackground(false, Vector2(-0.6, -0.3), UIColors[1]);
	CubemapSidesBox->SetMinSize(Vector2(0.3, 0.8));
	CubemapSidesBox->Align = UIBox::E_REVERSE;
	CubemapSidesBox->SetBorder(UIBox::E_ROUNDED, 1);
	CubemapSidesBox->IsVisible = false;
	PreviewWindow = new UIBackground(true, Vector2(-0.2, -0.3), 1, 0.8);
	PreviewWindow->SetBorder(UIBox::E_ROUNDED, 1);
	PreviewWindow->IsVisible = false;

}

void CubemapTab::Tick()
{
	try
	{
		PreviewWindow->IsVisible = TabBackground->IsVisible;
		CubemapSidesBox->IsVisible = TabBackground->IsVisible;
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
		throw(UIRenderingException("CubemapTab", e.what()));
	}
}

void CubemapTab::Load(std::string File)
{
	try
	{
		PreviewBuffer->GetBuffer()->ReInit(Graphics::WindowResolution.X, Graphics::WindowResolution.Y);
		Graphics::MainCamera->Position = Vector3(0, 4, 15);
		Graphics::MainCamera->SetRotation(Vector3(0, -90, 0));
		TabName->SetText("Cubemap: " + FileUtil::GetFileNameWithoutExtensionFromPath(File));
		delete SaveFile;
		InitialName = File;
		SaveFile = new SaveGame(InitialName.substr(0, InitialName.size() - 4), "cbm", false);

		UpdatePreviewModel();
	}
	catch (std::exception& e)
	{
		Log::Print("CUBEMAP: " + std::string(e.what()));
	}

	Generate();
}

void CubemapTab::Save()
{
	try
	{
		delete SaveFile;
		SaveFile = new SaveGame(InitialName.substr(0, InitialName.size() - 4), "cbm", false);
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}
}

void CubemapTab::Generate()
{
	CubemapSidesBox->DeleteChildren();
	SideFields.clear();
	for (size_t i = 0; i < Cubenames.size(); i++)
	{
		UITextField* text = new UITextField(true, 0, 0.2, this, 0, Renderer);
		text->SetText(SaveFile->GetPropterty(Cubenames[i]).Value);
		text->SetMinSize(Vector2(0.2, 0.05));
		text->SetPadding(0);
		text->SetBorder(UIBox::E_ROUNDED, 0.5);
		SideFields.push_back(text);
		CubemapSidesBox->AddChild((new UIBox(false, 0))
			->AddChild(text)
			->AddChild((new UIText(0.5, 1, DisplayNames[i], Renderer))
				->SetPadding(0, 0.01, 0, 0)));
	}
}

void CubemapTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible) return;
	switch (Index)
	{
	case 0:
		for (size_t i = 0; i < SideFields.size(); i++)
		{
			SaveFile->SetPropterty(SaveGame::SaveProperty(Cubenames[i], SideFields[i]->GetText(), Type::E_STRING));
		}
		Generate();
		break;
	default:
		break;
	}
	UpdatePreviewModel();
}

CubemapTab::~CubemapTab()
{
}

void CubemapTab::UpdateLayout()
{
	PreviewWindow->SetMinSize(Vector2(TabBackground->GetUsedSize().X * 0.5, TabBackground->GetUsedSize().X * 0.5));
}

void CubemapTab::UpdatePreviewModel()
{
	Save();
	ModelGenerator::ModelData m;
	m.MakeCube(30, 0, 0);
	m.Sphereize(150, -1);
	m.GenerateNormals(0);
	m.TwoSided = false;
	m.Materials[0] = "EditorContent/Materials/Reflective";
	PreviewModel = new Model(m);
	PreviewBuffer->ClearContent();
	PreviewBuffer->UseWith(PreviewModel);
	PreviewBuffer->ReflectionCubemap = Cubemap::LoadCubemapFile(FileUtil::GetFileNameWithoutExtensionFromPath(InitialName));
	PreviewModel->UpdateTransform();
}
#endif