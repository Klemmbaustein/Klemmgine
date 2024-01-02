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
		ParticleParam(&Element->Force, Type::Vector3, "Force"),
		ParticleParam(MaterialPtr, Type::String, "Material"),
		ParticleParam(&Element->StartScale, Type::Float, "Start scale"),
		ParticleParam(&Element->EndScale, Type::Float, "End scale"),
		ParticleParam(&Element->PositionRandom, Type::Vector3, "Position random"),
		ParticleParam(&Element->LifeTime, Type::Float, "Lifetime"),
		ParticleParam(&Element->NumLoops, Type::Int, "Particle count"),
		ParticleParam(&Element->SpawnDelay, Type::Float, "Particle count"),
	};

	Parameters.push_back(Params);
}

void ParticleEditorTab::GenerateElementButtons(const std::vector<ParticleParam>& ElementParams, UIBox* Target, int ElementIndex)
{
	float Size = 0.25f;
	int Index = ElementIndex * 100;
	std::vector<UIBox*> NewButtons;
	for (auto& i : ElementParams)
	{
		UIBox* New = nullptr;
		Target->AddChild((new UIText(0.45f, EditorUI::UIColors[2], i.Name, EditorUI::Text))
			->SetPadding(0.01f, 0, 0.01f, 0));
		switch (i.ParamType)
		{
		case Type::Vector3:
			New = new UIVectorField(Size, *(Vector3*)i.ValuePointer, this, Index, EditorUI::Text);
			Target->AddChild(New
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		case Type::String:
			New = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
				->SetText(*(std::string*)i.ValuePointer);
			Target->AddChild(New
				->SetMinSize(Vector2(Size, 0))
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		case Type::Float:
			New = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
				->SetText(EditorUI::ToShortString(*(float*)i.ValuePointer));
			Target->AddChild(New
				->SetMinSize(Vector2(Size, 0))
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		case Type::Int:
			New = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
				->SetText(std::to_string(*(int*)i.ValuePointer));
			Target->AddChild(New
				->SetMinSize(Vector2(Size, 0))
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		default:
			break;
		}
		Index++;
		NewButtons.push_back(New);
	}
	ParameterButtons.push_back(NewButtons);
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
	PreviewBackground = new UIBackground(UIBox::Orientation::Horizontal, 0, 1, 0.275f);
	PanelMainBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->SetPadding(0)
		->AddChild(ChildBox
			->SetPadding(0))
		->AddChild((new UIScrollBox(UIBox::Orientation::Vertical, 0, true))
			->AddChild(PreviewBackground
				->SetPadding(0.02f, 0, 0, 0))
			->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, -1))
				->SetPadding(0.02f, 0, 0, 0)
				->AddChild(new UIText(0.5f, 1 - EditorUI::UIColors[2], "New element", EditorUI::Text)))));

	if (!PreviewBuffer)
	{
		PreviewBuffer = new FramebufferObject();
		PreviewBuffer->UseMainWindowResolution = true;
		PreviewCamera = new Camera(2, 1600, 900);
		PreviewCamera->Position = Vector3(15, 10, 15);
		PreviewCamera->SetRotation(Vector3(0, -135, 0));
		PreviewBuffer->FramebufferCamera = PreviewCamera;
	}

	Load(File);
}

void ParticleEditorTab::Tick()
{
	PreviewBackground->SetUseTexture(true, PreviewBuffer->GetTextureID());
	if (PreviewBackground->IsVisibleInHierarchy())
	{
		UIBox::RedrawUI();
	}
}

void ParticleEditorTab::Load(std::string File)
{
	LoadedFile = File;
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
	PreviewBuffer->ParticleEmitters.push_back(Particle);

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
	Particles::ParticleEmitter::SaveToFile(Particle->ParticleElements, ElementMaterials, LoadedFile);
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
	ParameterButtons.clear();
	for (auto& i : Parameters)
	{
		UIBackground* ElementBackground = new UIBackground(UIBackground::Orientation::Vertical, 0, EditorUI::UIColors[0] * 0.75);
		ElementBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
			->SetPadding(0)
			->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "Element " + std::to_string(it) + ":", EditorUI::Text))
				->SetPadding(0.01f, 0, 0.01f, 0.2f))
			->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, it * 100 + 99))
				->SetUseTexture(true, EditorUI::Textures[4])
				->SetPadding(0)
				->SetSizeMode(UIBox::SizeMode::PixelRelative)
				->SetMinSize(0.04f)));

		Rows[it % NumRows]->AddChild(ElementBackground
			->SetPadding(0.02f, 0, 0, 0));

		GenerateElementButtons(i, ElementBackground, it);
		it++;
	}
}

void ParticleEditorTab::OnButtonClicked(int Index)
{
	HandlePanelButtons(Index);
	if (Index == -1)
	{
		Particle->AddElement(Particles::ParticleElement(), Material::LoadMaterialFile(""));
		ElementMaterials.push_back("");
		Generate();
		return;
	}
	if (Index >= 0)
	{
		if (Index % 100 == 99)
		{
			Particle->RemoveElement(Index / 100);
			ElementMaterials.erase(ElementMaterials.begin() + ((size_t)Index / 100));
			return;
		}

		ParticleParam* param = &Parameters[Index / 100][Index % 100];
		UIBox* Button = ParameterButtons[Index / 100][Index % 100];

		switch (param->ParamType)
		{
		case Type::Float:
			*(float*)param->ValuePointer = std::stof(((UITextField*)Button)->GetText());
			break;
		case Type::Vector3:
			*(Vector3*)param->ValuePointer = ((UIVectorField*)Button)->GetValue();
			break;
		case Type::String:
			*(std::string*)param->ValuePointer = ((UITextField*)Button)->GetText();
			break;
		case Type::Int:
			*(int*)param->ValuePointer = std::stoi(((UITextField*)Button)->GetText());
			break;
		default:
			break;
		}

		Particle->Reset();

		if (param->Name == "Material")
		{
			Particle->SetMaterial(Index / 100, Material::LoadMaterialFile(ElementMaterials[Index / 100]));
		}
		else
		{
			Generate();
		}
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
