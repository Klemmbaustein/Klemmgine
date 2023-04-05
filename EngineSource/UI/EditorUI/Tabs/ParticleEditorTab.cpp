#if EDITOR
#include "ParticleEditorTab.h"
#include <World/Stats.h>
#include <UI/UIButton.h>
#include <UI/EditorUI/UIVectorField.h>
#include <UI/UITextField.h>
#include <Engine/Log.h>
#include <World/Assets.h>
#include <filesystem>
#include <Rendering/Mesh/Model.h>
#include <UI/UIText.h>
#include <Engine/FileUtility.h>
#include <UI/UIScrollBox.h>
#include <Rendering/Mesh/Mesh.h>

void ParticleEditorTab::UpdateLayout()
{
	ParticleViewport->SetPosition(TabBackground->GetPosition());
	Generate();
}

ParticleEditorTab::ParticleEditorTab(Vector3* UIColors, TextRenderer* Text, unsigned int RemoveTexture, unsigned int ReloadTexture) : EditorTab(UIColors)
{
	ParticleFramebufferObject = new FramebufferObject();
	ParticleFramebufferObject->UseMainWindowResolution = true;
	Camera* Cam = new Camera(2, 1600, 900);
	Cam->Position = glm::vec3(15, 0, 40);
		Cam->yaw = 90;
	ParticleFramebufferObject->FramebufferCamera = Cam;
	
	this->RemoveTexture = RemoveTexture;
	Particle = new Particles::ParticleEmitter();
	Particle->SetMaterial(0, "NONE");
	ParticleViewport = new UIBackground(true, Vector2(-0.7, -0.6), 1, Vector2(0.4));
	ParticleFramebufferObject->ParticleEmitters.push_back(Particle);
	ParticleFramebufferObject->ReInit();
	ParticleViewport->IsVisible = false;
	ParticleViewportText = new UIText(0.4, Vector3(0, 0.5, 1), "Particle", Text);
	ParticleViewportText->SetPadding(0.01);
	SelectedElementText = new UIText(0.7, Vector3(1), "Particle has no elements", Text);
	SelectedElementText->IsVisible = false;
	SelectedElementText->SetPosition(Vector2(TabBackground->GetPosition() + TabBackground->GetUsedSize() * Vector2(0.5, 0.9)));
	ParticleViewport->AddChild(ParticleViewportText);
	TabText = Text;
	Generate();
}

void ParticleEditorTab::Tick()
{
	ParticleFramebufferObject->FramebufferCamera = Graphics::MainCamera;
	ParticleViewport->SetUseTexture(true, ParticleFramebufferObject->GetTextureID());
	ParticleViewport->IsVisible = TabBackground->IsVisible;

	for (auto* elem : SettingsButtons)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
	for (auto* elem : GeneratedUI)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
	ParticleSettingsScrollBox->IsVisible = TabBackground->IsVisible;
	SelectedElementText->IsVisible = TabBackground->IsVisible;
	if (TabBackground->IsVisible)
	{
		UIBox::RedrawUI();
	}
	ParticleViewportText->SetText("Particle: " + FileUtil::GetFileNameWithoutExtensionFromPath(CurrentSystemFile)
		+ " (" + std::to_string(Particle->ParticleElements.size()) + "/255 elements)");

	if (!Particle->IsActive)
	{
		ReactivateDelay -= Performance::DeltaTime;
		if (ReactivateDelay < 0)
		{
			Particle->Reset();
			ReactivateDelay = 1.f;
		}
	}
}

void ParticleEditorTab::Load(std::string File)
{
	CurrentSystemFile = File;
	Graphics::MainCamera->Position = Vector3(0, 4, 15);
	Graphics::MainCamera->SetRotation(Vector3(0, -90, 0));
	for (unsigned int i = 0; i < Particle->ParticleVertexBuffers.size(); i++)
	{
		delete Particle->ParticleVertexBuffers[i];
		delete Particle->ParticleIndexBuffers[i];
	}
	Particle->ParticleVertexBuffers.clear();
	Particle->ParticleIndexBuffers.clear();
	Particle->SpawnDelays.clear();
	Particle->ParticleShaders.clear();
	Particle->ParticleInstances.clear();
	Particle->Uniforms.clear();
	Particle->ParticleMatrices.clear();
	Particle->ParticleElements.clear();

	if (std::filesystem::exists(File))
	{
		if (!std::filesystem::is_empty(File))
		{
			auto ParticleData = Particles::ParticleEmitter::LoadParticleFile(File, ElementMaterials);
			for (unsigned int i = 0; i < ParticleData.size(); i++)
			{
				Particle->AddElement(ParticleData[i]);
				Particle->SetMaterial(i, ElementMaterials[i]);
			}
		}
	}
	Generate();
}

void ParticleEditorTab::ReloadMesh()
{
}

void ParticleEditorTab::Save()
{
	Particles::ParticleEmitter::SaveToFile(Particle->ParticleElements, ElementMaterials, CurrentSystemFile);
}

void ParticleEditorTab::Generate()
{
	Particle->Reset();
	for (auto* elem : SettingsButtons)
	{
		delete elem;
	}
	for (auto* elem : GeneratedUI)
	{
		delete elem;
	}
	GeneratedUI.clear();
	SelectedElementText->SetPosition(Vector2(TabBackground->GetPosition() + TabBackground->GetUsedSize() * Vector2(0.5, 1) - Vector2(0.1, 0.2)));
	for (unsigned int i = 0; i < Particle->ParticleElements.size(); i++)
	{
		Vector3 Color = i == SelectedElement ? Vector3(0.3f) : Vector3(0.15f);
		auto ButtonBox = new UIBox(true, TabBackground->GetPosition() + Vector2(0.05, TabBackground->GetUsedSize().Y - 0.2 - (i / 10.f)));
		auto NewButton = new UIButton(true, 0, Color, this, 200 + i);
		NewButton->SetMinSize(Vector2(0.2, 0));
		NewButton->SetBorder(UIBox::E_ROUNDED, 0.5);
		ButtonBox->AddChild(NewButton);
		NewButton->SetPadding(0, 0, 0, 0.01);
		auto RemoveButton = new UIButton(true, 0, 1, this, 300 + i);
		RemoveButton->SetMinSize(Vector2(0.09, 0.16) / 2.2);
		RemoveButton->SetUseTexture(true, RemoveTexture);
		RemoveButton->SetPadding(0);
		ButtonBox->AddChild(RemoveButton);
		GeneratedUI.push_back(NewButton);
		GeneratedUI.push_back(RemoveButton);
		NewButton->AddChild(new UIText(0.5, 1, "Element " + std::to_string(i), TabText));
	}
	if (Particle->ParticleElements.size())
	{
		SelectedElementText->SetText("Properties of 'Element " + std::to_string(SelectedElement) + "'");
	}
	else
	{
		SelectedElementText->SetText("Particle has no elements");
	}
	{
		auto NewButton = new UIButton(true, TabBackground->GetPosition() + Vector2(0.05, TabBackground->GetUsedSize().Y - 0.2 - (Particle->ParticleElements.size() / 10.f)), 1, this, 0);
		NewButton->SetBorder(UIBox::E_ROUNDED, 0.5);
		GeneratedUI.push_back(NewButton);
		NewButton->SetMinSize(Vector2(0.2, 0));
		NewButton->AddChild(new UIText(0.5, 0, "Add element", TabText));
	}
	SettingsButtons.clear();
	if (ParticleSettingsScrollBox) delete ParticleSettingsScrollBox;
	ParticleSettingsScrollBox = new UIScrollBox(true, TabBackground->GetPosition() + Vector2(TabBackground->GetUsedSize().X * 0.5 - 0.1, 0), 10);
	float ScrollSize = TabBackground->GetUsedSize().Y - 0.2;
	ParticleSettingsScrollBox->SetMaxSize(Vector2(0.6, ScrollSize));
	ParticleSettingsScrollBox->SetMinSize(Vector2(0, ScrollSize));
	UIBox* ScrollBoxes[2];
	for (auto& i : ScrollBoxes)
	{
		i = new UIBox(false, 0);
		i->Align = UIBox::E_REVERSE;
		i->SetPadding(0);
		i->SetMinSize(Vector2(0.3, ScrollSize));
		i->SetMaxSize(Vector2(0.3, ScrollSize));
		ParticleSettingsScrollBox->AddChild(i);
	}

	if (SelectedElement < Particle->ParticleElements.size())
	{
		auto& SelectedParticle = Particle->ParticleElements[SelectedElement];
		auto NewText = (new UIText(0.7, 1, "Velocity", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[0]->AddChild(NewText);
		UIVectorField* NewVecField = new UIVectorField(0, SelectedParticle.Direction, this, 100, TabText);
		ScrollBoxes[0]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7, 1, "Velocity random", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[0]->AddChild(NewText);
		NewVecField = new UIVectorField(0, SelectedParticle.DirectionRandom, this, 101, TabText);
		ScrollBoxes[0]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7, 1, "Size", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[0]->AddChild(NewText);
		UITextField* NewTextField = new UITextField(true, Vector2(-0.1f, 0.1f), 0.2, this, 102, TabText);
		ScrollBoxes[0]->AddChild(NewTextField);
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << SelectedParticle.Size;
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265, 0.05));
		NewTextField->SetBorder(UIBox::E_ROUNDED, 0.5);
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7, 1, "Lifetime", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[0]->AddChild(NewText);
		NewTextField = new UITextField(true, Vector2(-0.1f, 0.05f), 0.2, this, 103, TabText);
		ScrollBoxes[0]->AddChild(NewTextField);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.LifeTime;
		NewTextField->SetText(stream.str());
		NewTextField->SetBorder(UIBox::E_ROUNDED, 0.5);
		NewTextField->SetMinSize(Vector2(0.265, 0.05));
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7, 1, "Spawn delay", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[0]->AddChild(NewText);
		NewTextField = new UITextField(true, Vector2(-0.1f, -0.2f), 0.2, this, 104, TabText);
		ScrollBoxes[0]->AddChild(NewTextField);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.SpawnDelay;
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265, 0.05));
		NewTextField->SetBorder(UIBox::E_ROUNDED, 0.5);
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7, 1, "Spawn loops", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(true, Vector2(-0.1f, -0.35f), 0.2, this, 105, TabText);
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(std::to_string(SelectedParticle.NumLoops));
		NewTextField->SetMinSize(Vector2(0.265, 0.05));
		NewTextField->SetBorder(UIBox::E_ROUNDED, 0.5);
		SettingsButtons.push_back(NewTextField);

		NewText = (new UIText(0.7, 1, "Material", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(true, Vector2(-0.1f, -0.5f), 0.2, this, 106, TabText);
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(ElementMaterials[SelectedElement]);
		NewTextField->SetMinSize(Vector2(0.265, 0.05));
		NewTextField->SetBorder(UIBox::E_ROUNDED, 0.5);
		SettingsButtons.push_back(NewTextField);
		
		NewText = (new UIText(0.7, 1, "Position Random", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[1]->AddChild(NewText);
		NewVecField = new UIVectorField(0, SelectedParticle.PositionRandom, this, 107, TabText);
		ScrollBoxes[1]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7, 1, "Force", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[1]->AddChild(NewText);
		NewVecField = new UIVectorField(0, SelectedParticle.Force, this, 108, TabText);
		ScrollBoxes[1]->AddChild(NewVecField);
		SettingsButtons.push_back(NewVecField);

		NewText = (new UIText(0.7, 1, "Start scale", TabText))->SetPadding(0.01, 0.01, 0.01, 0.01);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(true, 0, 0.2, this, 109, TabText);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.StartScale;
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265, 0.05));
		NewTextField->SetBorder(UIBox::E_ROUNDED, 0.5);
		SettingsButtons.push_back(NewTextField);

		NewText = new UIText(0.7, 1, "End scale", TabText);
		ScrollBoxes[1]->AddChild(NewText);
		NewTextField = new UITextField(true, 0, 0.2, this, 110, TabText);
		stream = std::stringstream();
		stream << std::fixed << std::setprecision(2) << SelectedParticle.EndScale;
		ScrollBoxes[1]->AddChild(NewTextField);
		NewTextField->SetText(stream.str());
		NewTextField->SetMinSize(Vector2(0.265, 0.05));
		NewTextField->SetBorder(UIBox::E_ROUNDED, 0.5);
		SettingsButtons.push_back(NewTextField);
	}
	for (auto* elem : SettingsButtons)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
	for (auto* elem : GeneratedUI)
	{
		elem->IsVisible = TabBackground->IsVisible;
	}
}

void ParticleEditorTab::OnButtonClicked(int Index)
{
	if (!TabBackground->IsVisible) return;
	try
	{
		if (Index == 0)
		{
			ElementMaterials.push_back("NONE");
			Particle->AddElement(Particles::ParticleElement());
			Generate();
			return;
		}
		if (Index >= 300 && Index < 400)
		{
			Particle->RemoveElement(Index - 300);
			Generate();
			return;
		}
		if (Index >= 200 && Index < 300)
		{
			SelectedElement = Index - 200;
			Generate();
			return;
		}
		if (Index == 100)
		{
			Particle->ParticleElements[SelectedElement].Direction = ((UIVectorField*)SettingsButtons[Index - 100])->GetValue();
			Generate();
			return;
		}
		if (Index == 101)
		{
			Particle->ParticleElements[SelectedElement].DirectionRandom = ((UIVectorField*)SettingsButtons[Index - 100])->GetValue();
			Generate();
			return;
		}
		if (Index == 102)
		{
			Particle->ParticleElements[SelectedElement].Size = std::stof(((UITextField*)SettingsButtons[Index - 100])->GetText());
			Generate();
			return;
		}
		if (Index == 103)
		{
			Particle->ParticleElements[SelectedElement].LifeTime = std::stof(((UITextField*)SettingsButtons[Index - 100])->GetText());
			Generate();
			return;
		}
		if (Index == 104)
		{
			Particle->ParticleElements[SelectedElement].SpawnDelay = std::stof(((UITextField*)SettingsButtons[Index - 100])->GetText());
			Generate();
			return;
		}
		if (Index == 105)
		{
			Particle->ParticleElements[SelectedElement].NumLoops = std::stoi(((UITextField*)SettingsButtons[Index - 100])->GetText());
			Generate();
			return;
		}
		if (Index == 106)
		{
			Particle->SetMaterial(SelectedElement, ((UITextField*)SettingsButtons[Index - 100])->GetText());
			ElementMaterials[SelectedElement] = ((UITextField*)SettingsButtons[Index - 100])->GetText();
			Generate();
			return;
		}
		if (Index == 107)
		{
			Particle->ParticleElements[SelectedElement].PositionRandom = ((UIVectorField*)SettingsButtons[Index - 100])->GetValue();
			Generate();
			return;
		}
		if (Index == 108)
		{
			Particle->ParticleElements[SelectedElement].Force = ((UIVectorField*)SettingsButtons[Index - 100])->GetValue();
			Generate();
			return;
		}
		if (Index == 109)
		{
			Particle->ParticleElements[SelectedElement].StartScale = std::stof(((UITextField*)SettingsButtons[Index - 100])->GetText());
			Generate();
			return;
		}
		if (Index == 110)
		{
			Particle->ParticleElements[SelectedElement].EndScale = std::stof(((UITextField*)SettingsButtons[Index - 100])->GetText());
			Generate();
			return;
		}
	}
	catch (std::exception& e)
	{
		Log::Print(e.what());
	}
}

ParticleEditorTab::~ParticleEditorTab()
{
}
#endif