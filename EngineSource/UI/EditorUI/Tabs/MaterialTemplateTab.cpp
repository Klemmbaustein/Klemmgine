#if EDITOR
#pragma once
#include "MaterialTemplateTab.h"
#include "../MaterialFunctions.h"
#include <UI/EditorUI/UIVectorField.h>
#include <fstream>
#include <Engine/Log.h>
#include <UI/UIText.h>
#include <UI/UIScrollBox.h>
#include <UI/UIButton.h>
#include <Engine/FileUtility.h>

void MaterialTemplateTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible) return;
	if (Index == -1)
	{
		LoadedMaterial.Uniforms.push_back(Material::Param("", Type::E_INT, ""));
		GenerateUI();
	}
	else if (Index == -2)
	{
		LoadedMaterial.VertexShader = ShaderTextFields[0]->GetText();
		LoadedMaterial.FragmentShader = ShaderTextFields[1]->GetText();
	}
	else if (Index >= 0)
	{
		LoadedMaterial.Uniforms[Index].UniformName = ((UITextField*)TextFields[Index * 3])->GetText();
		if (LoadedMaterial.Uniforms[Index].Type != Type::E_VECTOR3)
		{
			LoadedMaterial.Uniforms[Index].Value = ((UITextField*)TextFields[Index * 3 + 2])->GetText();
		}
		else
		{
			LoadedMaterial.Uniforms[Index].Value = ((UIVectorField*)TextFields[Index * 3 + 2])->GetValue();
		}
		auto Type = MaterialUniformTypeStringToInt(((UITextField*)TextFields[Index * 3 + 1])->GetText());
		if (Type >= 0)
		{
			LoadedMaterial.Uniforms[Index].Type = Type;
		}
		GenerateUI();
	}
	else if (Index < -50 && Index >= -100)
	{
		LoadedMaterial.Uniforms.erase(LoadedMaterial.Uniforms.begin() + Index + 100);
		GenerateUI();
	}
}

void MaterialTemplateTab::Tick()
{
}

MaterialTemplateTab::MaterialTemplateTab(Vector3* UIColors, TextRenderer* Text, unsigned int XButtonIndex) : EditorTab(UIColors)
{
	XTexture = XButtonIndex;
	this->Renderer = Text;
	this->Renderer = Renderer;

	TabBackground->Align = UIBox::E_REVERSE;
	TabName = new UIText(1, 1, "Material Template: ", Renderer);
	TabName->SetPadding(0.1, 0.05, 0.05, 0);
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
}

void MaterialTemplateTab::Load(std::string File)
{
	Filepath = File;
	LoadedMaterial = Material();
	try
	{
		TabName->SetText("Material Template: " + FileUtil::GetFileNameWithoutExtensionFromPath(Filepath));
		LoadedMaterial = Material::LoadMaterialFile(Filepath, true);
		GenerateUI();
	}
	catch (std::exception& e)
	{
		Log::Print(e.what(), Vector3(0.8f, 0.f, 0.f));
	}

}

void MaterialTemplateTab::Save()
{
	Material::SaveMaterialFile(Filepath, LoadedMaterial, true);
}


void MaterialTemplateTab::GenerateUI()
{
	std::string ShaderHints[2] =
	{
		"Vertex shader",
		"Fragment shader"
	};
	std::string ShaderTexts[2] =
	{
		LoadedMaterial.VertexShader,
		LoadedMaterial.FragmentShader
	};
	int ButtonIndex = 0;
	Rows[0]->DeleteChildren();
	TextFields.clear();
	int it = 0;
	for (auto& i : ShaderTextFields)
	{
		Rows[0]->AddChild(new UIText(0.6, 0.8, ShaderHints[it], Renderer));
		i = new UITextField(true, 0, UIColors[1], this, -2, Renderer);
		i->HintText = ShaderHints[it] + " here";
		i->SetMinSize(Vector2(0.3, 0.075));
		i->SetMaxSize(Vector2(0.3, 0.075));
		i->SetText(ShaderTexts[it]);
		Rows[0] ->AddChild(i);
		it++;
	}
	Rows[0]->AddChild(new UIText(0.6, 0.8, "Uniforms", Renderer));
	auto ElementBox = new UIBox(true, 0);

	auto ElementNameText = new UIText(0.5, 1, "Uniform Name", Renderer);
	ElementNameText->SetPadding(0, 0, 0, 0.01);
	ElementNameText->SetTextWidthOverride(0.2);

	auto ElementTypeText = new UIText(0.5, 1, "Type", Renderer);
	ElementTypeText->SetPadding(0, 0, 0, 0.01);
	ElementTypeText->SetTextWidthOverride(0.1);

	UIText* ElementDefaultText = new UIText(0.5, 1, "Default Value", Renderer);
	ElementDefaultText->SetPadding(0, 0, 0, 0.01);
	ElementDefaultText->SetTextWidthOverride(0.3);

	ElementBox->AddChild(ElementNameText);
	ElementBox->AddChild(ElementTypeText);
	ElementBox->AddChild(ElementDefaultText);
	Rows[0]->AddChild(ElementBox);

	for (auto& i : LoadedMaterial.Uniforms)
	{
		ElementBox = new UIBox(true, 0);

		auto ElementName = new UITextField(true, 0, UIColors[1], this, ButtonIndex, Renderer);
		ElementName->SetText(i.UniformName);
		ElementName->HintText = "Uniform name";
		ElementName->SetMinSize(Vector2(0.2, 0.075));
		ElementName->SetPadding(0, 0, 0, 0.01);
		ElementName->SetMaxSize(Vector2(0.3, 0.075));

		auto ElementType = new UITextField(true, 0, UIColors[1], this, ButtonIndex, Renderer);
		ElementType->SetText(Type::Types[i.Type]);
		ElementType->HintText = "Type";
		ElementType->SetMinSize(Vector2(0.1, 0.075));
		ElementType->SetPadding(0, 0, 0, 0.01);
		ElementType->SetMaxSize(Vector2(0.3, 0.075));

		
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

		UIButton* DeleteButton = new UIButton(true, 0, 1, this, ButtonIndex - 100);
		DeleteButton->SetPadding(0, 0, 0, 0.01);
		DeleteButton->SetUseTexture(true, XTexture);
		DeleteButton->SetMinSize(Vector2(0.9 / 24, 1.6 / 24));
		ElementBox->AddChild(ElementName);
		TextFields.push_back(ElementName);
		ElementBox->AddChild(ElementType);
		TextFields.push_back(ElementType);
		ElementBox->AddChild(ElementDefaultValue);
		TextFields.push_back(ElementDefaultValue);
		ElementBox->AddChild(DeleteButton);
		Rows[0]->AddChild(ElementBox);
		ButtonIndex++;
	}
	auto AddNewButton = new UIButton(true, 0, Vector3(0, 1, 0), this, -1);
	auto AddNewText = new UIText(0.6, 0, "Add new", Renderer);
	AddNewButton->AddChild(AddNewText);
	Rows[0]->AddChild(AddNewButton);
}
#endif