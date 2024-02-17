#if EDITOR
#include "MaterialTab.h"
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
#include <Math/Math.h>
#include <UI/UIDropdown.h>
#include <Rendering/Mesh/Model.h>

void MaterialTab::OnButtonClicked(int Index)
{
	if (!PanelMainBackground->IsVisible)
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

		switch (LoadedMaterial.Uniforms.at(Index).NativeType)
		{
		case NativeType::Int:
		case NativeType::Float:
			Value = ((UITextField*)TextFields[Index])->GetText();
			break;
		case NativeType::GL_Texture:
		{
			Texture::TextureInfo Info;
			Info.File = ((UITextField*)TextFields[Index])->GetText();
			Info.Filtering = (Texture::TextureFiltering)((UIDropdown*)TextureDropdowns.at(Index * 2))->SelectedIndex;
			Info.Wrap = (Texture::TextureWrap)((UIDropdown*)TextureDropdowns.at(Index * 2 + 1))->SelectedIndex;
			Value = Texture::CreateTextureInfoString(Info);
			break;
		}
		case NativeType::Vector3:
		case NativeType::Vector3Color:
			Value = ((UIVectorField*)TextFields[Index])->GetValue().ToString();
			break;
		case NativeType::Bool:
			Value = (Value == "1") ? "0" : "1";
			break;
		default:
			break;
		}
		GenerateUI();
	}
	HandlePanelButtons(Index);
}

MaterialTab::MaterialTab(EditorPanel* Parent, std::string File) : EditorTab(Parent, "Material", File)
{
	auto RowBox = new UIBox(UIBox::Orientation::Horizontal, 0);
	PanelMainBackground->AddChild(RowBox
		->SetPadding(0));
	Rows[0] = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	RowBox->AddChild(Rows[0]
		->SetPadding(0));

	Rows[1] = new UIBackground(UIBox::Orientation::Vertical, 0, EditorUI::UIColors[0] * 0.75f);
	RowBox->AddChild(Rows[1]
		->SetPadding(0));


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

MaterialTab::~MaterialTab()
{
	delete PreviewBuffer;
	delete PreviewModel;
	delete PreviewCamera;
}

void MaterialTab::Tick()
{
	TickPanel();
	PreviewCamera->ReInit(1.7f, Graphics::WindowResolution.X, Graphics::WindowResolution.Y);
	if (!PreviewWindow)
	{
		return;
	}
	PreviewBuffer->Active = PanelMainBackground->IsVisible;
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
		LoadedMaterial = Material::LoadMaterialFile(Filepath);

		std::string Path = "Shaders/" + LoadedMaterial.FragmentShader;
		Path = Path.substr(0, Path.find_last_of("/\\"));

		std::vector<Material::Param> ShaderUniforms = Preprocessor::ParseGLSL(
			FileUtil::GetFileContent("Shaders/" + LoadedMaterial.FragmentShader),
			Path).ShaderParams;
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
					|| LoadedMaterial.Uniforms[i].NativeType != ShaderUniforms[i].NativeType)
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
void MaterialTab::OnResized()
{
	Rows[0]->SetMinSize(Vector2(Scale.X - 0.3f, Scale.Y));
	Rows[0]->SetMaxSize(Vector2(Scale.X - 0.3f, Scale.Y));
	Rows[1]->SetMinSize(Vector2(0.3f, Scale.Y));
	GenerateUI();
	GenerateMaterialProperties();
}

void MaterialTab::Save()
{
	if (Filepath.empty())
	{
		return;
	}
	Material::SaveMaterialFile(Filepath, LoadedMaterial);
	Material::ReloadMaterial(Filepath);
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
	TextureDropdowns.clear();
	Rows[0]->AddChild((new UIText(0.6f, EditorUI::UIColors[2], "Uniforms:", EditorUI::Text))
		->SetPadding(0.05f, 0.01f, 0.01f, 0.0f));
	float VerticalSize = Rows[0]->GetMinSize().X * 0.95f;
	float ValueSize = VerticalSize / 2.5f;
	float DescriptionSize = VerticalSize - ValueSize - 0.05f;
	ValueSize *= 0.95f;
	float DesiredValueSize = std::min(ValueSize, 0.3f);

	int Index = 0;
	for (auto& i : LoadedMaterial.Uniforms)
	{
		auto ParamBox = new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0]);

		size_t CategorySeperator = i.Description.find_first_of("#");

		std::string Category = i.Description.substr(0, CategorySeperator - 1);
		std::string Description = i.Description.substr(CategorySeperator + 1);

		auto TextBox = (new UIBox(UIBox::Orientation::Vertical, 0));

		ParamBox->AddChild(TextBox
			->SetMinSize(Vector2(DescriptionSize, 0.1f))
			->SetPadding(0.005f)
			->AddChild((new UIText(0.5f, EditorUI::UIColors[2], i.UniformName, EditorUI::MonoText))
				->SetWrapEnabled(true, DescriptionSize * 1.6f, UIBox::SizeMode::ScreenRelative)
				->SetPadding(0.005f))
			->AddChild((new UIText(0.4f, Vector3::Lerp(EditorUI::UIColors[2], 0.5f, 0.25f), Description, EditorUI::Text))
				->SetWrapEnabled(true, DescriptionSize * 1.6f, UIBox::SizeMode::ScreenRelative)
				->SetPadding(0.005f)));

		ParamBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[0] * 0.75f, Vector2(2.0f / Graphics::WindowResolution.X, 0)))
			->SetTryFill(true)
			->SetPadding(0.01f, 0.01f, 0, 0));

		UIBox* NewField = nullptr;

		switch (i.NativeType)
		{
		case NativeType::Vector3:
		{	
			NewField = (new UIVectorField(DesiredValueSize, Vector3::FromString(i.Value), this, Index, EditorUI::Text))
				->SetValueType(UIVectorField::VecType::rgb);
			break;
		}
		case NativeType::Float:
		case NativeType::Int:
		{
			NewField = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
				->SetText(i.Value)
				->SetTextColor(EditorUI::UIColors[2])
				->SetMinSize(Vector2(DesiredValueSize, 0.04f));
			break;
		}
		case NativeType::Bool:
		{
			NewField = new UIButton(UIBox::Orientation::Horizontal, 0, 1, this, Index);
			NewField->SetSizeMode(UIBox::SizeMode::PixelRelative);
			NewField->SetMinSize(0.04f);
			NewField->SetBorder(UIBox::BorderType::Rounded, 0.3f);
			NewField->SetPadding(0.03f, 0.03f, 0.02f, 0.01f);
			if (i.Value == "1")
			{
				((UIButton*)NewField)->SetUseTexture(true, EditorUI::Textures[16]);
			}
			break;
		}
		case NativeType::GL_Texture:
		{
			auto TextureInfo = Texture::ParseTextureInfoString(i.Value);
			unsigned int NewTexture = Texture::LoadTexture(TextureInfo);
			NewField = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
				->SetTextColor(EditorUI::UIColors[2])
				->SetText(TextureInfo.File)
				->SetPadding(0.02f, 0.02f, 0, 0)
				->SetMinSize(Vector2(DesiredValueSize - 0.1f, 0.04f));
			ParamBox->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, 1, DesiredValueSize > 0.175f ? 0.15f : 0.1f))
				->SetUseTexture(true, NewTexture)
				->SetSizeMode(UIBox::SizeMode::PixelRelative));

			UIBox* TextureSelectionBoxes[2] =
			{
				(new UIDropdown(0, 0.09f, EditorUI::UIColors[1], EditorUI::UIColors[2],
				{
					UIDropdown::Option("Nearest"),
					UIDropdown::Option("Linear"),
				}, Index, this, EditorUI::Text))
				->SetTextSize(0.4f, 0.001f)
				->SelectOption((int)TextureInfo.Filtering)
				->SetMinSize(Vector2((DesiredValueSize - 0.11f) / 2.0f, 0))
				->SetPadding(0, 0.02f, 0, 0.01f),
				
				(new UIDropdown(0, 0.09f, EditorUI::UIColors[1], EditorUI::UIColors[2],
				{
					UIDropdown::Option("Clamp"),
					UIDropdown::Option("Border"),
					UIDropdown::Option("Repeat"),
				}, Index, this, EditorUI::Text))
				->SetTextSize(0.4f, 0.001f)
				->SelectOption((int)TextureInfo.Wrap)
				->SetMinSize(Vector2((DesiredValueSize - 0.11f) / 2.0f, 0))
				->SetPadding(0, 0.02f, 0, 0)
			};

			ParamBox->AddChild((new UIBox(UIBox::Orientation::Vertical, 0))
				->SetPadding(0)
				->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
					->SetPadding(0)
					->AddChild(NewField))
				->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
					->SetPadding(0)
					->AddChild((new UIBox(UIBox::Orientation::Vertical, 0))
						->SetPadding(0)
						->AddChild((new UIText(0.35f, EditorUI::UIColors[2] * 0.75f, "Filtering:", EditorUI::Text))
							->SetPadding(0))
						->AddChild(TextureSelectionBoxes[0]))
					->AddChild((new UIBox(UIBox::Orientation::Vertical, 0))
						->SetPadding(0)
						->AddChild((new UIText(0.35f, EditorUI::UIColors[2] * 0.75f, "Wrap:", EditorUI::Text))
							->SetPadding(0))
						->AddChild(TextureSelectionBoxes[1]))));

			TextBox->SetMinSize(Vector2(DescriptionSize, 0.15f));
			PreviewTextures.push_back(NewTexture);
			TextureDropdowns.push_back(TextureSelectionBoxes[0]);
			TextureDropdowns.push_back(TextureSelectionBoxes[1]);
			break;
		}
		default:
			break;
		}
		Index++;
		if (NewField && i.NativeType != NativeType::GL_Texture)
		{
			ParamBox->AddChild(NewField);
			TextureDropdowns.push_back(nullptr);
			TextureDropdowns.push_back(nullptr);
		}
		TextFields.push_back(NewField);
		
		Rows[0]->AddChild(ParamBox
			->SetBorder(UIBox::BorderType::DarkenedEdge, 0.2f)
			->SetMinSize(Vector2(VerticalSize, 0))
			->SetPadding(0.005f));
	}

	if (LoadedMaterial.Uniforms.empty())
	{
		Rows[0]->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "No parameters in shader.", EditorUI::Text))
			->SetPadding(0.005f));
		Rows[0]->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "Add parameters using // #params", EditorUI::Text))
			->SetPadding(0.005f));
	}

	UpdateModel();
}

void MaterialTab::GenerateMaterialProperties()
{
	Rows[1]->DeleteChildren();
	PreviewWindow = new UIBackground(UIBox::Orientation::Vertical, 0, 1, 0.25f);
	auto ScrollBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	Rows[1]->AddChild(ScrollBox
		->SetPadding(0)
		->SetMaxSize(Rows[1]->GetMinSize())
		->SetMinSize(Rows[1]->GetMinSize()));

	ScrollBox->AddChild(PreviewWindow
		->SetUseTexture(PreviewBuffer->GetTextureID()));

	ScrollBox->AddChild(new UIText(0.5f, EditorUI::UIColors[2], "Shader files", EditorUI::Text));

	ShaderTextFields[0] = new UITextField(0, EditorUI::UIColors[0], this, -4, EditorUI::Text);
	ShaderTextFields[1] = new UITextField(0, EditorUI::UIColors[0], this, -4, EditorUI::Text);

	ScrollBox->AddChild(ShaderTextFields[0]
		->SetTextColor(EditorUI::UIColors[2])
		->SetText(LoadedMaterial.VertexShader)
		->SetPadding(0.005f, 0.005f, 0.02f, 0.02f)
		->SetMinSize(Vector2(0.2f, 0)));

	ScrollBox->AddChild(ShaderTextFields[1]
		->SetTextColor(EditorUI::UIColors[2])
		->SetText(LoadedMaterial.FragmentShader)
		->SetPadding(0.005f, 0.02f, 0.02f, 0.02f)
		->SetMinSize(Vector2(0.2f, 0)));

	std::vector<std::pair<std::string, bool>> Options =
	{
		std::pair("Is transparent", LoadedMaterial.IsTranslucent),
		std::pair("Use shadow cutout", LoadedMaterial.UseShadowCutout),
	};

	int it = 0;
	for (const auto& i : Options)
	{
		auto NewField = new UIButton(UIBox::Orientation::Horizontal, 0, 0.75f, this, -5 - it++);
		NewField->SetSizeMode(UIBox::SizeMode::PixelRelative);
		NewField->SetMinSize(0.04f);
		NewField->SetBorder(UIBox::BorderType::Rounded, 0.3f);
		NewField->SetPadding(0, 0, 0.03f, 0);
		if (i.second)
		{
			((UIButton*)NewField)->SetUseTexture(true, EditorUI::Textures[16]);
		}

		ScrollBox->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
			->SetPadding(0.01f, 0.00, 0.02f, 0.02f)
			->AddChild((new UIText(0.5f, EditorUI::UIColors[2], i.first, EditorUI::Text))
				->SetTextWidthOverride(0.15f)
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