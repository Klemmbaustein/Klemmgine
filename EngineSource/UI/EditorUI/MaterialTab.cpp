#if 0
#if EDITOR
#pragma once
#include "MaterialTab.h"
#include "MaterialFunctions.h"
#include <fstream>
#include <filesystem>
#include <Engine/Log.h>
#include <World/Assets.h>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <Engine/FileUtility.h>


void MaterialTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible) return;
	if (Index == -2)
	{
		if (LoadedMaterial.Template != ParentTemplateText->GetText())
		{
			if (std::filesystem::exists(Assets::GetAsset(ParentTemplateText->GetText() + ".jsmtmp")))
			{
				LoadedMaterial.Template = ParentTemplateText->GetText();
				LoadTemplate(LoadedMaterial.Template);
			}
		}
	}
	else if (Index == -3)
	{
		Save();
	}
	else if (Index == -4)
	{
		FetchTemplate(LoadedMaterial.Template);
		Log::Print("Fetched Template " + LoadedMaterial.Template + " for File " + Filepath);
	}
	else if (Index == -5)
	{
		LoadedMaterial.UseShadowCutout = !LoadedMaterial.UseShadowCutout;
		CutoutText->SetText(LoadedMaterial.UseShadowCutout ? "true" : "false");
	}
	else if (Index == -6)
	{
		LoadedMaterial.IsTranslucent = !LoadedMaterial.IsTranslucent;
		TranslucencyText->SetText(LoadedMaterial.IsTranslucent ? "true" : "false");
	}
	else if (Index >= 0)
	{
		if (LoadedMaterial.Uniforms.at(Index).Type != Type::E_VECTOR3)
		{
			LoadedMaterial.Uniforms.at(Index).Value = ((UITextField*)TextFields[Index])->GetText();
		}
		else
		{
			LoadedMaterial.Uniforms.at(Index).Value = ((UIVectorField*)TextFields[Index])->GetValue().ToString();
		}
	}
}

MaterialTab::MaterialTab(Vector3* UIColors, TextRenderer* Text, unsigned int ReloadTexture) : EditorTab(UIColors)
{
	Renderer = Text;
	this->ReloadTexture = ReloadTexture;

	TabBackground->Align = UIBox::E_REVERSE;
	TabName = new UIText(1, 1, "Material: " + GetFileNameWithoutExtensionFromPath(Filepath), Renderer);
	TabName->SetPadding(0.05, 0, 0.05, 0);
	TabBackground->AddChild(TabName);
	auto RowBox = new UIBox(true, 0);
	TabBackground->AddChild(RowBox);
	Rows[0] = new UIScrollBox(false, 0, 25);
	Rows[0]->SetMaxSize(Vector2(999, 1.1));
	RowBox->AddChild(Rows[0]);
	Rows[0]->Align = UIBox::E_REVERSE;

	Rows[1] = new UIBox(false, 0);
	Rows[1]->Align = UIBox::E_REVERSE;
	RowBox->AddChild(Rows[1]);

	RightRow = new UIBox(false, 0.3);

	UIBox* OptionBoxes[2];
	OptionBoxes[0] = new UIBox(true, 0);
	OptionBoxes[1] = new UIBox(true, 0);
	auto NewText = new UIText(0.6, 1, "Use shadow cutout:", Renderer);
	OptionBoxes[0]->AddChild(NewText);
	NewText->SetPadding(0);
	auto NewButton = new UIButton(true, 0, UIColors[0] * 2, this, -5);
	OptionBoxes[0]->AddChild(NewButton);
	OptionBoxes[0]->SetPadding(0.01);
	NewButton->SetPadding(0, 0, 0.01, 0);
	CutoutText = new UIText(0.6, 1, "false", Renderer);
	CutoutText->SetPadding(0.01);
	NewButton->AddChild(CutoutText);


	NewText = new UIText(0.6, 1, "Is translucent:", Renderer);
	OptionBoxes[1]->AddChild(NewText);
	NewText->SetPadding(0);

	NewButton = new UIButton(true, 0, UIColors[0] * 2, this, -6);
	OptionBoxes[1]->AddChild(NewButton);
	OptionBoxes[1]->SetPadding(0.01);
	NewButton->SetPadding(0, 0, 0.01, 0);

	TranslucencyText = new UIText(0.6, 1, "false", Renderer);
	TranslucencyText->SetPadding(0.01);
	NewButton->AddChild(TranslucencyText);


	auto NewBox = new UIBox(true, 0);
	NewBox->SetPadding(0);
	NewButton = new UIButton(true, 0, 1, this, -4);
	NewButton->SetMinSize(Vector2(0.9, 1.6) / 30);
	NewText = new UIText(0.6, 1, " Reload template", Renderer);
	NewText->SetPadding(0);
	NewButton->SetPadding(0, 0, 0.02, 0);
	NewButton->SetUseTexture(true, ReloadTexture);
	ShaderTexts[0] = new UIText(0.5, 0.8, "", Renderer);
	ShaderTexts[1] = new UIText(0.5, 0.8, "", Renderer);
	ShaderTexts[0]->SetPadding(0.02, 0, 0, 0);
	ShaderTexts[1]->SetPadding(0);
	RightRow->AddChild(ShaderTexts[1]);
	RightRow->AddChild(ShaderTexts[0]);
	NewBox->AddChild(NewText);
	NewBox->AddChild(NewButton);
	RightRow->AddChild(NewBox);
	RightRow->AddChild(OptionBoxes[0]);
	RightRow->AddChild(OptionBoxes[1]);
	RightRow->IsVisible = TabBackground->IsVisible;
	ShaderTexts[0]->SetText("Vertex Shader: " + LoadedMaterial.VertexShader);
	ShaderTexts[1]->SetText("Fragment (Pixel) Shader: " + LoadedMaterial.FragmentShader);
	GenerateUI();
}
void MaterialTab::Tick()
{
	RightRow->IsVisible = TabBackground->IsVisible;
}


void MaterialTab::Load(std::string File)
{
	Filepath = File;
	LoadedMaterial = Material();
	try
	{
		TabName->SetText("Material: " + GetFileNameWithoutExtensionFromPath(Filepath));
		LoadedMaterial = Material::LoadMaterialFile(Filepath, false);
		GenerateUI();
	}
	catch (std::exception& e)
	{
		Log::Print(e.what(), Vector3(0.8f, 0.f, 0.f));
	}
}

void MaterialTab::LoadTemplate(std::string Template)
{
	LoadedMaterial = Material();	
	Material LoadedTemplate = Material::LoadMaterialFile(Template, true);

	LoadedMaterial = LoadedTemplate;
	LoadedMaterial.Template = Template;

	GenerateUI();
}

void MaterialTab::FetchTemplate(std::string Template)
{
	Template = Template + ".jsmtmp";
	Template = Assets::GetAsset(Template);
	if (std::filesystem::exists(Template))
	{
		auto FetchedTemplate = Material::LoadMaterialFile(Template, true);
		
		for (auto& i : FetchedTemplate.Uniforms)
		{
			for (auto& j : LoadedMaterial.Uniforms)
			{
				if (i.UniformName == j.UniformName && i.Type == j.Type)
				{
					i.Value = j.Value;
				}
			}
		}

		LoadedMaterial.Uniforms = FetchedTemplate.Uniforms;
		LoadedMaterial.FragmentShader = FetchedTemplate.FragmentShader;
		LoadedMaterial.VertexShader = FetchedTemplate.VertexShader;
		Save();
	}
	GenerateUI();
}

void MaterialTab::Save()
{
	Material::SaveMaterialFile(Filepath, LoadedMaterial, false);
}

void MaterialTab::GenerateUI()
{

	ShaderTexts[0]->SetText("Vertex Shader: " + LoadedMaterial.VertexShader);
	ShaderTexts[1]->SetText("Fragment (Pixel) Shader: " + LoadedMaterial.FragmentShader);
	int ButtonIndex = 0;
	Rows[0]->DeleteChildren();
	TextFields.clear();

	TranslucencyText->SetText(LoadedMaterial.IsTranslucent ? "true" : "false");
	CutoutText->SetText(LoadedMaterial.UseShadowCutout ? "true" : "false");

	Rows[0]->AddChild(new UIText(0.8, 0.9, "Parent template", Renderer));
	ParentTemplateText = new UITextField(true, 0, UIColors[1], this, -2, Renderer);
	ParentTemplateText->HintText = "Parent Template";
	ParentTemplateText->SetMinSize(Vector2(0.3, 0.075));
	ParentTemplateText->SetMaxSize(Vector2(0.3, 0.075));
	ParentTemplateText->SetText(LoadedMaterial.Template);
	Rows[0]->AddChild(ParentTemplateText);

	Rows[0]->AddChild(new UIText(0.675, 0.9, "Uniforms", Renderer));
	auto ElementBox = new UIBackground(true, 0, UIColors[1] * 1.5, Vector2(0.8, 0));

	auto ElementNameText = new UIText(0.6, 1, "Uniform name", Renderer);
	ElementNameText->SetPadding(0.02, 0.02, 0.02, 0.01);
	ElementNameText->SetTextWidthOverride(0.27);

	auto ElementTypeText = new UIText(0.6, 1, "Type", Renderer);
	ElementTypeText->SetPadding(0.02, 0.02, 0, 0.01);
	ElementTypeText->SetTextWidthOverride(0.09);

	UIText* ElementDefaultText = new UIText(0.6, 1, "Default value", Renderer);
	ElementDefaultText->SetPadding(0.02, 0.02, 0, 0.01);
	ElementDefaultText->SetTextWidthOverride(0.3);

	ElementBox->AddChild(ElementNameText);
	ElementBox->AddChild(ElementTypeText);
	ElementBox->AddChild(ElementDefaultText);
	Rows[0]->AddChild(ElementBox);
	for (auto& i : LoadedMaterial.Uniforms)
	{
		ElementBox = new UIBackground(true, 0, UIColors[1] * 1.5, Vector2(0.675, 0));

		auto ElementNameBox = new UIBox(true, 0);
		auto ElementName = new UIText(0.6, 1, i.UniformName, Renderer);
		ElementNameBox->SetMinSize(Vector2(0.3, 0.075));
		ElementNameBox->AddChild(ElementName);
		ElementNameBox->SetPadding(0);
		ElementName->SetPadding(0, 0.02, 0.02, 0.01);
		ElementName->SetMaxSize(Vector2(0.3, 0.075));

		auto ElementTypeBox = new UIBox(true, 0);
		auto ElementType = new UIText(0.6, 1, Type::Types[i.Type], Renderer);
		ElementTypeBox->SetMinSize(Vector2(0.1, 0.075));
		ElementType->SetPadding(0, 0.02, 0, 0.01);
		ElementTypeBox->SetPadding(0);
		ElementType->SetMaxSize(Vector2(0.3, 0.075));
		ElementTypeBox->AddChild(ElementType);

		UIBox* ElementDefaultValue = nullptr;
		if (i.Type != Type::E_VECTOR3)
		{
			ElementDefaultValue = new UITextField(true, 0, UIColors[1], this, ButtonIndex, Renderer);
			((UITextField*)ElementDefaultValue)->SetText(i.Value);
			((UITextField*)ElementDefaultValue)->HintText = "Default value";
			ElementDefaultValue->SetPadding(0, 0, 0, 0.01);
			ElementDefaultValue->SetMinSize(Vector2(0.265, 0.075));
			ElementDefaultValue->SetMaxSize(Vector2(0.265, 0.075));
		}
		else
		{
			ElementDefaultValue = new UIVectorField(0, Vector3::stov(i.Value), this, ButtonIndex, Renderer);
			ElementDefaultValue->SetPadding(0, 0.01, 0, 0.01);
		}

		ElementBox->AddChild(ElementNameBox);
		ElementBox->AddChild(ElementTypeBox);
		ElementBox->AddChild(ElementDefaultValue);
		TextFields.push_back(ElementDefaultValue);
		Rows[0]->AddChild(ElementBox);
		ButtonIndex++;
	}
}

#endif
#endif