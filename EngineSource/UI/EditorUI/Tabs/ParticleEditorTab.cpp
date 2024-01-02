#if EDITOR
#include "ParticleEditorTab.h"
#include <Engine/Stats.h>
#include <UI/UIButton.h>
#include <UI/EditorUI/UIVectorField.h>
#include <UI/UITextField.h>
#include <Engine/Log.h>
#include <Engine/File/Assets.h>
#include <filesystem>
#include <Rendering/Mesh/Model.h>
#include <UI/UIText.h>
#include <Engine/Utility/FileUtility.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Mesh.h>
#include <UI/EditorUI/EditorUI.h>

void ParticleEditorTab::AddParametersForElement(Particles::ParticleElement* Element, std::string* MaterialPtr)
{
	std::vector<ParticleParam> Params = 
	{
		ParticleParam(&Element->Direction, Type::Vector3, "Start velocity"),
		ParticleParam(&Element->DirectionRandom, Type::Vector3, "Velocity random"),
		ParticleParam(MaterialPtr, Type::String, "Material"),
		ParticleParam(&Element->StartScale, Type::Float, "Start scale"),
		ParticleParam(&Element->EndScale, Type::Float, "End scale"),
	};

	Parameters.push_back(Params);
}

void ParticleEditorTab::GenerateElementButtons(const std::vector<ParticleParam>& ElementParams, UIBox* Target)
{
	float Size = 0.25f;
	for (auto& i : ElementParams)
	{
		Target->AddChild((new UIText(0.4f, EditorUI::UIColors[2], i.Name, EditorUI::Text))
			->SetPadding(0.01f, 0, 0.01f, 0));
		switch (i.ParamType)
		{
		case Type::Vector3:
			Target->AddChild((new UIVectorField(Size, *(Vector3*)i.ValuePointer, this, 0, EditorUI::Text))
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		case Type::String:
			Target->AddChild((new UITextField(0, EditorUI::UIColors[1], this, 0, EditorUI::Text))
				->SetText(*(std::string*)i.ValuePointer)
				->SetMinSize(Vector2(Size, 0))
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		case Type::Float:
			Target->AddChild((new UITextField(0, EditorUI::UIColors[1], this, 0, EditorUI::Text))
				->SetText(EditorUI::ToShortString(*(float*)i.ValuePointer))
				->SetMinSize(Vector2(Size, 0))
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		default:
			break;
		}
	}
}

void ParticleEditorTab::OnResized()
{
	ChildBox->SetMinSize(Vector2(Scale.X - 0.3f, Scale.Y - 0.005f));
	ChildBox->SetMaxSize(Vector2(Scale.X - 0.3f, Scale.Y - 0.005f));
	Generate();
}

ParticleEditorTab::ParticleEditorTab(EditorPanel* Parent, std::string File) : EditorTab(Parent, "Particle", File)
{
	Particle = new Particles::ParticleEmitter();
	ChildBox = new UIScrollBox(UIBox::Orientation::Horizontal, 0, true);
	PanelMainBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->SetPadding(0)
		->AddChild(ChildBox
			->SetPadding(0))
		->AddChild(new UIBackground(UIBox::Orientation::Horizontal, 0, 1, 0.2f)));
	Load(File);
}

void ParticleEditorTab::Tick()
{
}

void ParticleEditorTab::Load(std::string File)
{
	CurrentSystemFile = File;
	for (unsigned int i = 0; i < Particle->ParticleVertexBuffers.size(); i++)
	{
		delete Particle->ParticleVertexBuffers[i];
	}
	Particle->ParticleVertexBuffers.clear();
	Particle->SpawnDelays.clear();
	Particle->ParticleInstances.clear();
	Particle->Contexts.clear();
	Particle->ParticleMatrices.clear();
	Particle->ParticleElements.clear();

	if (std::filesystem::exists(File) && !std::filesystem::is_empty(File))
	{
		auto ParticleData = Particles::ParticleEmitter::LoadParticleFile(File, ElementMaterials);
		for (unsigned int i = 0; i < ParticleData.size(); i++)
		{
			Particle->AddElement(ParticleData[i], Material::LoadMaterialFile(ElementMaterials[i]));
		}
	}
	Generate();
}

void ParticleEditorTab::Save()
{
	Particles::ParticleEmitter::SaveToFile(Particle->ParticleElements, ElementMaterials, CurrentSystemFile);
}

void ParticleEditorTab::Generate()
{
	Parameters.clear();
	for (size_t i = 0; i < ElementMaterials.size(); i++)
	{
		AddParametersForElement(&Particle->ParticleElements[i], &ElementMaterials[i]);
	}

	ChildBox->DeleteChildren();
	std::vector<UIBox*> Rows;
	int NumRows = std::max((int)(ChildBox->GetMinSize().X / 0.3f), 1);
	for (int i = 0; i < NumRows; i++)
	{
		UIBox* New = new UIBox(UIBox::Orientation::Vertical, 0);
		ChildBox->AddChild(New);

		Rows.push_back(New);
	}

	int it = 0;
	for (auto& i : Parameters)
	{
		UIBackground* ElementBackground = new UIBackground(UIBackground::Orientation::Vertical, 0, EditorUI::UIColors[0] * 0.75);
		ElementBackground->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "Element " + std::to_string(it) + ":", EditorUI::Text))
			->SetPadding(0.01f));
		Rows[it % NumRows]->AddChild(ElementBackground
			->SetPadding(0.02f, 0, 0, 0));
		GenerateElementButtons(i, ElementBackground);
		it++;
	}

	Rows[0]->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, -1))
		->SetPadding(0.02f, 0, 0, 0)
		->AddChild(new UIText(0.5f, 1 - EditorUI::UIColors[2], "New element", EditorUI::Text)));
}

void ParticleEditorTab::OnButtonClicked(int Index)
{
	if (Index == -1)
	{
		Particle->AddElement(Particles::ParticleElement(), Material::LoadMaterialFile(""));
		ElementMaterials.push_back("");
		Generate();
		return;
	}
}

ParticleEditorTab::~ParticleEditorTab()
{
}


ParticleEditorTab::ParticleParam::ParticleParam(void* ValuePointer, Type::TypeEnum ParamType, std::string Name)
{
	this->ValuePointer = ValuePointer;
	this->ParamType = ParamType;
	this->Name = Name;
}

#endif
