#if EDITOR
#pragma once
#include "MaterialTab.h"
#include "../MaterialFunctions.h"
#include <fstream>
#include <filesystem>
#include <Engine/Log.h>
#include <Engine/File/Assets.h>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Engine/Utility/FileUtility.h>
#include <Rendering/Utility/ShaderPreprocessor.h>
#include <UI/EditorUI/EditorUI.h>
#include <Rendering/Texture/Texture.h>
#include <cmath>
#include <Rendering/Mesh/Model.h>

void MaterialTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible)
	{
		return;
	}
	if (Index == -3)
	{
		Save();
	}
	else if (Index == -4)
	{
		if (LoadedMaterial.VertexShader != ShaderTextFields[0]->GetText())
		{
			LoadedMaterial.VertexShader = ShaderTextFields[0]->GetText();
			Save();
			Load(Filepath);
		}
		if (LoadedMaterial.FragmentShader != ShaderTextFields[1]->GetText())
		{
			LoadedMaterial.FragmentShader = ShaderTextFields[1]->GetText();
			Save();
			Load(Filepath);
		}
	}
	else if (Index == -5)
	{
		LoadedMaterial.IsTranslucent = !LoadedMaterial.IsTranslucent;
		GenerateMaterialProperties();
	}
	else if (Index == -6)
	{
		LoadedMaterial.UseShadowCutout = !LoadedMaterial.UseShadowCutout;
		GenerateMaterialProperties();
	}
	else if (Index >= 0)
	{
		if (TextFields[Index] == nullptr)
		{
			return;
		}

		auto& Value = LoadedMaterial.Uniforms.at(Index).Value;

		switch (LoadedMaterial.Uniforms.at(Index).Type)
		{
		case Type::Int:
		case Type::Float:
		case Type::GL_Texture:
			Value = ((UITextField*)TextFields[Index])->GetText();
			break;
		case Type::Vector3:
		case Type::Vector3Color:
			Value = ((UIVectorField*)TextFields[Index])->GetValue().ToString();
			break;
		case Type::Bool:
			Value = (Value == "1") ? "0" : "1";
			break;
		default:
			break;
		}
		GenerateUI();
	}
}

MaterialTab::MaterialTab(Vector3* UIColors, TextRenderer* Text) : EditorTab(UIColors)
{
	Renderer = Text;

	TabBackground->SetAlign(UIBox::Align::Reverse);
	TabName = new UIText(1, UIColors[2], "Material: " + FileUtil::GetFileNameWithoutExtensionFromPath(Filepath), Renderer);
	TabName->SetPadding(0.1f, 0.05f, 0.05f, 0);
	TabBackground->AddChild(TabName);
	auto RowBox = new UIBox(true, 0);
	TabBackground->AddChild(RowBox);
	Rows[0] = new UIScrollBox(false, 0, true);
	RowBox->AddChild(Rows[0]);
	Rows[0]->SetAlign(UIBox::Align::Reverse);

	Rows[1] = new UIBackground(false, 0, UIColors[1]);
	Rows[1]->SetAlign(UIBox::Align::Reverse);
	RowBox->AddChild(Rows[1]);


	if (!PreviewBuffer)
	{
		PreviewBuffer = new FramebufferObject();
		PreviewBuffer->UseMainWindowResolution = true;
		PreviewCamera = new Camera(1.7f, 1600, 900);
		PreviewCamera->Position = Vector3(-6, 8, 6);
		PreviewCamera->yaw = -45;
		PreviewCamera->pitch = -45;
		PreviewBuffer->FramebufferCamera = PreviewCamera;

		UpdateModel();
	}

	GenerateUI();
}

void MaterialTab::Tick()
{
	PreviewCamera->ReInit(1.7f, Graphics::WindowResolution.X, Graphics::WindowResolution.Y);
	if (!PreviewWindow)
	{
		return;
	}
	PreviewBuffer->Active = TabBackground->IsVisible;
	PreviewWindow->SetUseTexture(true, PreviewBuffer->GetTextureID());
	if (RedrawFrames)
	{
		RedrawFrames--;
		if (RedrawFrames == 0)
		{
			UIBox::RedrawUI();
		}
	}
}


void MaterialTab::Load(std::string File)
{

	Filepath = File;
	LoadedMaterial = Material();
	try
	{
		TabName->SetText("Material: " + FileUtil::GetFileNameWithoutExtensionFromPath(Filepath));
		LoadedMaterial = Material::LoadMaterialFile(Filepath);

		std::string Path = "Shaders/" + LoadedMaterial.FragmentShader;
		Path = Path.substr(0, Path.find_last_of("/\\"));

		std::vector<Material::Param> ShaderUniforms = Preprocessor::ParseGLSL(FileUtil::GetFileContent("Shaders/" + LoadedMaterial.FragmentShader), Path).ShaderParams;
		for (size_t i = 0; i < std::max(ShaderUniforms.size(), LoadedMaterial.Uniforms.size()); i++)
		{
			if (ShaderUniforms.size() <= i)
			{
				LoadedMaterial.Uniforms.erase(LoadedMaterial.Uniforms.begin() + i, LoadedMaterial.Uniforms.end());
				break;
			}
			if (LoadedMaterial.Uniforms.size() <= i)
			{
				LoadedMaterial.Uniforms.push_back(ShaderUniforms[i]);
			}
			else if (LoadedMaterial.Uniforms[i].UniformName != ShaderUniforms[i].UniformName
					|| LoadedMaterial.Uniforms[i].Type != ShaderUniforms[i].Type)
			{
				LoadedMaterial.Uniforms[i] = ShaderUniforms[i];
			}
			else
			{
				LoadedMaterial.Uniforms[i].Description = ShaderUniforms[i].Description;
			}
		}

		GenerateUI();
	}
	catch (std::exception& e)
	{
		Log::Print(e.what(), Vector3(0.8f, 0.f, 0.f));
	}
}
void MaterialTab::UpdateLayout()
{
	Rows[0]->SetMinSize(Vector2(TabBackground->GetMinSize().X / 1.75f, TabBackground->GetMinSize().Y - 0.275f));
	Rows[0]->SetMaxSize(Vector2(TabBackground->GetMinSize().X / 1.75f, TabBackground->GetMinSize().Y - 0.275f));
	Rows[1]->SetMinSize(Vector2(0, TabBackground->GetMinSize().Y - 0.275f));
	GenerateMaterialProperties();
}

void MaterialTab::Save()
{
	Material::SaveMaterialFile(Filepath, LoadedMaterial);
}

void MaterialTab::GenerateUI()
{
	Rows[0]->DeleteChildren();
	TextFields.clear();
	for (unsigned int i : PreviewTextures)
	{
		Texture::UnloadTexture(i);
	}
	PreviewTextures.clear();

	int Index = 0;
	for (auto& i : LoadedMaterial.Uniforms)
	{
		auto ParamBox = new UIBackground(true, 0, UIColors[1]);

		size_t CategorySeperator = i.Description.find_first_of("#");

		std::string Category = i.Description.substr(0, CategorySeperator - 1);
		std::string Description = i.Description.substr(CategorySeperator + 1);

		auto TextBox = (new UIBox(false, 0));

		ParamBox->AddChild(TextBox
			->SetMinSize(Vector2(0.3f, 0.1f))
			->SetPadding(0.005f)
			->SetAlign(UIBox::Align::Reverse)
			->AddChild((new UIText(0.5f, UIColors[2], i.UniformName, Renderer))
				->SetWrapEnabled(true, 0.4f, UIBox::SizeMode::ScreenRelative)
				->SetPadding(0.005f))
			->AddChild((new UIText(0.4f, Vector3::Lerp(UIColors[2], 0.5f, 0.25f), Description, Renderer))
				->SetWrapEnabled(true, 0.5f, UIBox::SizeMode::ScreenRelative)
				->SetPadding(0.005f)));

		UIBox* NewField = nullptr;

		switch (i.Type)
		{
		case Type::Vector3:
		{	
			NewField = (new UIVectorField(0, Vector3::stov(i.Value), this, Index, Renderer))
				->SetValueType(UIVectorField::VecType::rgb);
			break;
		}
		case Type::Float:
		case Type::Int:
		{
			NewField = (new UITextField(true, 0, UIColors[0], this, Index, Renderer))
				->SetText(i.Value)
				->SetTextColor(UIColors[2])
				->SetMinSize(Vector2(0.26f, 0.05f));
			break;
		}
		case Type::Bool:
		{
			NewField = new UIButton(true, 0, 0.75f, this, Index);
			NewField->SetSizeMode(UIBox::SizeMode::PixelRelative);
			NewField->SetMinSize(0.04f);
			NewField->SetBorder(UIBox::BorderType::Rounded, 0.3f);
			NewField->SetPadding(0.03f, 0.03f, 0.02f, 0.01f);
			if (i.Value == "1")
			{
				((UIButton*)NewField)->SetUseTexture(true, Editor::CurrentUI->Textures[16]);
			}
			break;
		}
		case Type::GL_Texture:
		{
			unsigned int NewTexture = Texture::LoadTexture(i.Value);
			NewField = (new UITextField(true, 0, UIColors[0], this, Index, Renderer))
				->SetTextColor(UIColors[2])
				->SetText(i.Value)
				->SetPadding(0.02f, 0.02f, 0, 0)
				->SetMinSize(Vector2(0.2f, 0.05f));
			ParamBox->AddChild((new UIBackground(true, 0, 1, 0.15f))
				->SetUseTexture(true, NewTexture)
				->SetSizeMode(UIBox::SizeMode::PixelRelative));
			TextBox->SetMinSize(Vector2(0.3f, 0.15f));
			PreviewTextures.push_back(NewTexture);
			break;
		}
		default:
			break;
		}
		Index++;
		if (NewField)
		{
			ParamBox->AddChild(NewField);
		}
		TextFields.push_back(NewField);
		
		Rows[0]->AddChild(ParamBox
			->SetMinSize(Vector2(0.65f, 0))
			->SetPadding(0.005f));
	}

	if (LoadedMaterial.Uniforms.empty())
	{
		Rows[0]->AddChild((new UIText(0.5f, UIColors[2], "No parameters in shader.", Renderer))
			->SetPadding(0.005f));
		Rows[0]->AddChild((new UIText(0.5f, UIColors[2], "Add parameters using // #params", Renderer))
			->SetPadding(0.005f));
	}

	UpdateModel();
}

void MaterialTab::GenerateMaterialProperties()
{
	Rows[1]->DeleteChildren();
	PreviewWindow = new UIBackground(true, 0, 1, 0.3f);
	Rows[1]->AddChild(PreviewWindow
		->SetUseTexture(PreviewBuffer->GetTextureID()));

	Rows[1]->AddChild(new UIText(0.6f, 1, "Shader files", Renderer));

	ShaderTextFields[0] = new UITextField(true, 0, UIColors[0], this, -4, Renderer);
	ShaderTextFields[1] = new UITextField(true, 0, UIColors[0], this, -4, Renderer);

	Rows[1]->AddChild(ShaderTextFields[0]
		->SetText(LoadedMaterial.VertexShader)
		->SetPadding(0.005f, 0.005f, 0.02f, 0.02f)
		->SetMinSize(Vector2(0.2f, 0)));

	Rows[1]->AddChild(ShaderTextFields[1]
		->SetText(LoadedMaterial.FragmentShader)
		->SetPadding(0.005f, 0.02f, 0.02f, 0.02f)
		->SetMinSize(Vector2(0.2f, 0)));

	std::vector<std::pair<std::string, bool>> Options =
	{
		std::pair("Is transparent   ", LoadedMaterial.IsTranslucent),
		std::pair("Use shadow cutout", LoadedMaterial.UseShadowCutout),
	};

	int it = 0;
	for (const auto& i : Options)
	{
		auto NewField = new UIButton(true, 0, 0.75f, this, -5 - it++);
		NewField->SetSizeMode(UIBox::SizeMode::PixelRelative);
		NewField->SetMinSize(0.04f);
		NewField->SetBorder(UIBox::BorderType::Rounded, 0.3f);
		NewField->SetPadding(0, 0, 0.03f, 0);
		if (i.second)
		{
			((UIButton*)NewField)->SetUseTexture(true, Editor::CurrentUI->Textures[16]);
		}

		Rows[1]->AddChild((new UIBox(true, 0))
			->SetPadding(0.01f, 0.00, 0.02f, 0.02f)
			->AddChild((new UIText(0.5f, UIColors[2], i.first, Renderer))
				->SetPadding(0))
			->AddChild(NewField));
	}
}

void MaterialTab::UpdateModel()
{
	Save();
	PreviewBuffer->ClearContent();
	PreviewModel = nullptr;

	if (!std::filesystem::exists(Filepath))
	{
		return;
	}

	ModelGenerator::ModelData m;
	m.Elements.push_back(ModelGenerator::ModelData::Element());
	auto& elem = m.Elements[m.Elements.size() - 1];
	elem.MakeCube(30, 0);
	elem.ElemMaterial = Filepath;
	elem.GenerateNormals();
	m.TwoSided = false;

	PreviewModel = new Model(m);
	PreviewModel->ModelTransform.Rotation.X = Math::PI_F;
	PreviewBuffer->UseWith(PreviewModel);
	PreviewModel->UpdateTransform();
	RedrawFrames = 2;
}

#endif