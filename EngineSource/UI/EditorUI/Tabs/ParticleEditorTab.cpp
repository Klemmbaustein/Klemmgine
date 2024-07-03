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
#include <Objects/ParticleObject.h>

void ParticleEditorTab::AddParametersForElement(Particles::ParticleElement* Element, std::string* MaterialPtr)
{
	std::vector<ParticleParam> Params =
	{
		ParticleParam(&Element->Direction, NativeType::Vector3, "Start velocity"),
		ParticleParam(&Element->DirectionRandom, NativeType::Vector3, "Velocity random"),
		ParticleParam(&Element->Force, NativeType::Vector3, "Force"),
		ParticleParam(MaterialPtr, NativeType::String, "Material"),
		ParticleParam(&Element->StartScale, NativeType::Float, "Start scale"),
		ParticleParam(&Element->EndScale, NativeType::Float, "End scale"),
		ParticleParam(&Element->PositionRandom, NativeType::Vector3, "Position random"),
		ParticleParam(&Element->LifeTime, NativeType::Float, "Lifetime"),
		ParticleParam(&Element->NumLoops, NativeType::Int, "Particle count"),
		ParticleParam(&Element->SpawnDelay, NativeType::Float, "Spawn delay"),
	};

	Parameters.push_back(Params);
}

void ParticleEditorTab::GenerateElementButtons(const std::vector<ParticleParam>& ElementParams, UIBox* Target, int ElementIndex)
{
	float Size = std::min(Scale.X - 0.1f, 0.25f);
	int Index = ElementIndex * 100;
	std::vector<UIBox*> NewButtons;
	for (auto& i : ElementParams)
	{
		UIBox* New = nullptr;
		Target->AddChild((new UIText(0.45f, EditorUI::UIColors[2], i.Name, EditorUI::Text))
			->SetPadding(0.01f, 0, 0.01f, 0));
		switch (i.ParamType)
		{
		case NativeType::Vector3:
			New = new UIVectorField(Size, *(Vector3*)i.ValuePointer, this, Index, EditorUI::Text);
			Target->AddChild(New
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		case NativeType::String:
			New = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
				->SetText(*(std::string*)i.ValuePointer);
			Target->AddChild(New
				->SetMinSize(Vector2(Size, 0))
				->SetPadding(0, 0.01f, 0.01f, 0));

			if (i.Name == "Material")
			{
				((UITextField*)New)->SetTextColor(Assets::GetAsset(*(std::string*)i.ValuePointer + ".jsmat").empty()
					? Vector3(1.0, 0.0f, 0.0f) 
					: EditorUI::UIColors[2]);
			}
			break;
		case NativeType::Float:
			New = (new UITextField(0, EditorUI::UIColors[1], this, Index, EditorUI::Text))
				->SetText(EditorUI::ToShortString(*(float*)i.ValuePointer));
			Target->AddChild(New
				->SetMinSize(Vector2(Size, 0))
				->SetPadding(0, 0.01f, 0.01f, 0));
			break;
		case NativeType::Int:
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
	if (Scale.X >= 0.6)
	{
		ChildBox->SetMinSize(Vector2(Scale.X - 0.3f, Scale.Y - 0.005f));
		ChildBox->SetMaxSize(Vector2(Scale.X - 0.3f, Scale.Y - 0.005f));
		SideBar->SetMinSize(Vector2(0.3f, Scale.Y));
		SideBar->SetMaxSize(Vector2(0.3f, Scale.Y));
		SideBar->IsVisible = true;
	}
	else
	{
		ChildBox->SetMinSize(Vector2(Scale.X, Scale.Y - 0.005f));
		ChildBox->SetMaxSize(Vector2(Scale.X, Scale.Y - 0.005f));
		SideBar->SetMinSize(Vector2(0));
		SideBar->SetMaxSize(Vector2(0));
		SideBar->IsVisible = false;
	}

	Generate();
}

ParticleEditorTab::ParticleEditorTab(EditorPanel* Parent, std::string File) : EditorTab(Parent, "Particle", File)
{
	Particle = new Particles::ParticleEmitter();
	ChildBox = new UIScrollBox(UIBox::Orientation::Horizontal, 0, true);
	SideBar = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);

	PreviewBackground = new UIBackground(UIBox::Orientation::Horizontal, 0, 1, 0.275f);
	PanelMainBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
		->SetPadding(0)
		->AddChild(ChildBox
			->SetPadding(0))
		->AddChild(SideBar
			->SetPadding(0)
			->AddChild(PreviewBackground
				->SetPadding(0.02f, 0, 0, 0))
			->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, -1))
				->SetPadding(0.02f, 0, 0, 0)
				->AddChild((new UIText(0.5f, 1 - EditorUI::UIColors[2], "New element", EditorUI::Text))
					->SetPadding(0.01f)))));

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
		PreviewBackground->RedrawElement();
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

	for (SceneObject* o : Objects::AllObjects)
	{
		ParticleObject* ParticleObj = dynamic_cast<ParticleObject*>(o);
		if (ParticleObj && ParticleObj->ParticleName == FileUtil::GetFileNameWithoutExtensionFromPath(LoadedFile))
		{
			ParticleObj->LoadParticle(ParticleObj->ParticleName);
		}
	}
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
		New->SetMinSize(Vector2(std::min(ChildBox->GetMinSize().X - 0.1f, 0.325f), 0));
		ChildBox->AddChild(New);

		Rows.push_back(New
			->SetPadding(0.02f));
	}

	int it = 0;
	ParameterButtons.clear();
	for (auto& i : Parameters)
	{
		UIBackground* ElementBackground = new UIBackground(UIBackground::Orientation::Vertical, 0, EditorUI::UIColors[0] * 0.75);
		ElementBackground->AddChild((new UIBox(UIBox::Orientation::Horizontal, 0))
			->SetPadding(0)
			->AddChild((new UIText(0.5f, EditorUI::UIColors[2], "Element " + std::to_string(it) + ":", EditorUI::Text))
				->SetTextWidthOverride(std::min(0.25f, Scale.X - 0.1f))
				->SetPadding(0.01f, 0, 0.01f, 0))
			->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, it * 100 + 99))
				->SetUseTexture(true, EditorUI::Textures[4])
				->SetPadding(0)
				->SetSizeMode(UIBox::SizeMode::AspectRelative)
				->SetMinSize(0.04f)));

		Rows[it % NumRows]->AddChild(ElementBackground
			->SetPadding(0.02f, 0, 0, 0));

		GenerateElementButtons(i, ElementBackground, it);
		it++;
	}

	if (Scale.X < 0.6)
	{
		Rows[it % NumRows]->AddChild((new UIButton(UIBox::Orientation::Horizontal, 0, EditorUI::UIColors[2], this, -1))
			->SetPadding(0.02f, 0, 0, 0)
			->AddChild((new UIText(0.5f, 1 - EditorUI::UIColors[2], "New element", EditorUI::Text))
				->SetPadding(0.01f)));
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
		Save();
		return;
	}
	if (Index >= 0)
	{
		if (Index % 100 == 99)
		{
			Particle->RemoveElement(Index / 100);
			ElementMaterials.erase(ElementMaterials.begin() + ((size_t)Index / 100));
			Generate();
			Save();
			return;
		}

		ParticleParam* param = &Parameters[Index / 100][Index % 100];
		UIBox* Button = ParameterButtons[Index / 100][Index % 100];

		try
		{
			switch (param->ParamType)
			{
			case NativeType::Float:
				*(float*)param->ValuePointer = std::stof(((UITextField*)Button)->GetText());
				break;
			case NativeType::Vector3:
				*(Vector3*)param->ValuePointer = ((UIVectorField*)Button)->GetValue();
				break;
			case NativeType::String:
				*(std::string*)param->ValuePointer = ((UITextField*)Button)->GetText();
				break;
			case NativeType::Int:
				*(int*)param->ValuePointer = std::stoi(((UITextField*)Button)->GetText());
				break;
			default:
				break;
			}
		}
		catch (std::exception)
		{

		}

		Particle->Reset();

		if (param->Name == "Material")
		{
			Particle->SetMaterial(Index / 100, Material::LoadMaterialFile(ElementMaterials[Index / 100]));
		}
		Generate();
		Save();
	}
}

ParticleEditorTab::~ParticleEditorTab()
{
	delete PreviewBuffer;
	delete PreviewCamera;
	delete Particle;
}

ParticleEditorTab::ParticleParam::ParticleParam(void* ValuePointer, NativeType::NativeType ParamType, std::string Name)
{
	this->ValuePointer = ValuePointer;
	this->ParamType = ParamType;
	this->Name = Name;
}

#endif
