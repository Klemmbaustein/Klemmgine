#ifdef EDITOR
#pragma once
#include "CubemapTab.h"
#include <fstream>
#include <Engine/Utility/FileUtility.h>
#include <Rendering/Mesh/ModelGenerator.h>
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Utility/Framebuffer.h>
#include <UI/UIButton.h>
#include <Rendering/Texture/Cubemap.h>
#include <Engine/File/Save.h>

CubemapTab::CubemapTab(Vector3* UIColors, TextRenderer* Renderer) : EditorTab(UIColors)
{
	this->Renderer = Renderer;

	TabBackground->Align = UIBox::E_REVERSE;
	TabName = new UIText(1, UIColors[2], "Model: ", Renderer);
	TabName->SetPadding(0.1f, 0.05f, 0.05f, 0);
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
	CubemapSidesBox = new UIBackground(false, Vector2(-0.6f, -0.3f), UIColors[1]);
	CubemapSidesBox->SetMinSize(Vector2(0.3f, 0.8f));
	CubemapSidesBox->Align = UIBox::E_REVERSE;
	CubemapSidesBox->SetBorder(UIBox::E_ROUNDED, 1);
	CubemapSidesBox->IsVisible = false;
	PreviewWindow = new UIBackground(true, Vector2(-0.2f, -0.3f), 0.999f, 0.8f);
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
		PreviewBuffer->Active = TabBackground->IsVisible;
		PreviewBuffer->ClearContent();
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
		PreviewBuffer->GetBuffer()->ReInit((int)Graphics::WindowResolution.X, (int)Graphics::WindowResolution.Y);
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
		UITextField* text = new UITextField(true, 0, 0.2f, this, 0, Renderer);
		text->SetText(SaveFile->GetPropterty(Cubenames[i]).Value);
		text->SetMinSize(Vector2(0.2f, 0.05f));
		text->SetPadding(0);
		text->SetBorder(UIBox::E_ROUNDED, 0.5f);
		SideFields.push_back(text);
		CubemapSidesBox->AddChild((new UIBox(false, 0))
			->AddChild(text)
			->AddChild((new UIText(0.5f, UIColors[2], DisplayNames[i], Renderer))
				->SetPadding(0, 0.01f, 0, 0)));
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
			SaveFile->SetPropterty(SaveGame::SaveProperty(Cubenames[i], SideFields[i]->GetText(), Type::String));
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
	PreviewWindow->SetMinSize(Vector2(TabBackground->GetUsedSize().X * 0.5f, TabBackground->GetUsedSize().X * 0.5f));
}

void CubemapTab::UpdatePreviewModel()
{
	Save();
	ModelGenerator::ModelData m;
	m.Elements.push_back(ModelGenerator::ModelData::Element());
	auto& elem = m.Elements[m.Elements.size() - 1];
	elem.MakeCube(30, 0);
	elem.Sphereize(150);
	elem.GenerateNormals();
	m.TwoSided = false;
	elem.ElemMaterial = "../../EditorContent/Materials/Reflective";
	PreviewModel = new Model(m);
	PreviewBuffer->ClearContent();
	PreviewBuffer->UseWith(PreviewModel);
	PreviewBuffer->ReflectionCubemap = Cubemap::LoadCubemapFile(FileUtil::GetFileNameWithoutExtensionFromPath(InitialName));
	PreviewModel->UpdateTransform();
}
#endif