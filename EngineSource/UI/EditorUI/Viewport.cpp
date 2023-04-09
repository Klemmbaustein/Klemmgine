#if EDITOR
#include "Viewport.h"
#include <Engine/Input.h>
#include <UI/EditorUI/EditorUI.h>
#include <Math/Math.h>
#include <Engine/Application.h>
#include <Objects/Components/MeshComponent.h>
#include <Objects/Components/InstancedMeshComponent.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Mesh/InstancedModel.h>
#include <Math/Collision/Collision.h>
#include <Engine/Log.h>
#include <Rendering/Mesh/Model.h>
#include <UI/EditorUI/Tabs/MeshTab.h>
#include <UI/EditorUI/Tabs/MaterialTab.h>
#include <UI/EditorUI/Tabs/MaterialTemplateTab.h>
#include <UI/EditorUI/Tabs/ParticleEditorTab.h>
#include <UI/EditorUI/Tabs/CubemapTab.h>
#include <Engine/FileUtility.h>


Viewport* Viewport::ViewportInstance = nullptr;

// Collision model for the arrows



Collision::Box ArrowBoxX
(
	0.0f, 1.0f,
	-0.1f, 0.1f,
	-0.1f, 0.1f

);

Collision::Box ArrowBoxY
(
	-0.1f, 0.1f,
	0.0f, 1.0f,
	-0.1f, 0.1f

);

Collision::Box ArrowBoxZ
(
	-0.1f, 0.1f,
	-0.1f, 0.1f,
	-1.0f, 0.0f

);

Viewport::Viewport(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorPanel(Colors, Position, Scale, 0, 2)
{
	ViewportInstance = this;

	OutlineBuffer = new FramebufferObject();
	OutlineBuffer->FramebufferCamera = Graphics::MainCamera;
	ArrowsBuffer = new FramebufferObject();
	ArrowsBuffer->FramebufferCamera = Graphics::MainCamera;
	ArrowsModel = new Model("EditorContent/Models/Arrows.jsm");
	ArrowsModel->ModelTransform.Scale = Vector3(1, 1, -1);
	ArrowsModel->ModelTransform.Rotation.Y = -M_PI_2;
	ArrowsBuffer->Renderables.push_back(ArrowsModel);
	TabBackground->IsVisible = false;

	TabInstances = 
	{
		nullptr,
		new MeshTab(Editor::CurrentUI->UIColors, Editor::CurrentUI->EngineUIText),
		new MaterialTab(Editor::CurrentUI->UIColors, Editor::CurrentUI->EngineUIText, Editor::CurrentUI->Textures[12]),
		new MaterialTemplateTab(Editor::CurrentUI->UIColors, Editor::CurrentUI->EngineUIText, Editor::CurrentUI->Textures[4]),
		new ParticleEditorTab(Editor::CurrentUI->UIColors, Editor::CurrentUI->EngineUIText, Editor::CurrentUI->Textures[4], Editor::CurrentUI->Textures[12]),
		new CubemapTab(Editor::CurrentUI->UIColors, Editor::CurrentUI->EngineUIText)
	};

	TabBox = new UIBackground(true, Position + Vector2(0, Scale.Y - 0.05), UIColors[1], Vector2(Scale.X, 0.05));
	UpdateTabBar();
}

void Viewport::ClearSelectedObjects()
{
	for (auto i : SelectedObjects)
	{
		i->IsSelected = false;
	}
	SelectedObjects.clear();
}


void Viewport::UpdateLayout()
{
	UpdateTabBar();
	for (size_t i = 0; i < TabInstances.size(); i++)
	{
		if (Tabs[SelectedTab].Index == i && TabInstances[i])
		{
			TabInstances[i]->TabBackground->SetPosition(Position);
			TabInstances[i]->TabBackground->SetMinSize(Scale);
			TabInstances[i]->TabBackground->SetMaxSize(Scale);
			TabInstances[i]->TabBackground->IsVisible = true;
			TabInstances[i]->UpdateLayout();
		}
		else if (TabInstances[i])
		{
			TabInstances[i]->TabBackground->IsVisible = false;
		}
	}
}
void Viewport::Tick()
{
	bool TabHas3DView = !TabInstances[Tabs[SelectedTab].Index] || Tabs[SelectedTab].Index == 1 || Tabs[SelectedTab].Index == 4 || Tabs[SelectedTab].Index == 5;
	Graphics::MainCamera->FOV = Maths::PI / 1.2;
	UpdatePanel();

	OutlineBuffer->Renderables.clear();
	OutlineBuffer->FramebufferCamera = Graphics::MainCamera;
	ArrowsBuffer->FramebufferCamera = Graphics::MainCamera;

	ArrowsModel->Visible = SelectedObjects.size();
	if (SelectedObjects.size())
	{
		ArrowsModel->ModelTransform.Location = SelectedObjects[0]->GetTransform().Location;
		ArrowsModel->ModelTransform.Scale = Vector3(1, 1, -1) * Vector3::Distance(Graphics::MainCamera->Position, ArrowsModel->ModelTransform.Location) * 0.03f;
		ArrowsModel->UpdateTransform();
	}

	SelectedObjects.clear();
	for (auto i : Objects::AllObjects)
	{
		if (!i->IsSelected) continue;
		SelectedObjects.push_back(i);

		for (auto j : i->GetComponents())
		{
			if (dynamic_cast<MeshComponent*>(j))
			{
				OutlineBuffer->Renderables.push_back(dynamic_cast<MeshComponent*>(j)->GetModel());
			}
			if (dynamic_cast<InstancedMeshComponent*>(j))
			{
				OutlineBuffer->Renderables.push_back(dynamic_cast<InstancedMeshComponent*>(j)->GetInstancedModel());
			}
		}
	}
	if (PreviousSelectedObject && !SelectedObjects.size())
	{
		PreviousSelectedObject = nullptr;
		Editor::CurrentUI->UIElements[5]->UpdateLayout();
		Editor::CurrentUI->UIElements[6]->UpdateLayout();
	}
	else if (SelectedObjects.size() && PreviousSelectedObject != SelectedObjects[0])
	{
		PreviousSelectedObject = SelectedObjects[0];
		Editor::CurrentUI->UIElements[5]->UpdateLayout();
		Editor::CurrentUI->UIElements[6]->UpdateLayout();
	}

	if (Input::IsKeyDown(SDLK_ESCAPE))
	{
		ClearSelectedObjects();
	}

	auto Viewport = Editor::CurrentUI->UIElements[4];
	Vector2 RelativeMouseLocation = Application::GetCursorPosition() - (Viewport->Position + (Viewport->Scale * 0.5));
	Vector3 Rotation = Graphics::MainCamera->ForwardVectorFromScreenPosition(RelativeMouseLocation.X, RelativeMouseLocation.Y);

	if (Maths::IsPointIn2DBox(Viewport->Position, Viewport->Position + Viewport->Scale, Input::MouseLocation)
		&& !Dragging
		&& TabHas3DView)
	{

		if (!Editor::CurrentUI->CurrentCursor && !TabInstances[Tabs[SelectedTab].Index]) // Default Cursor = 0. So if the current cursor evaluates to 'false' its the default cursor
		{
			Editor::CurrentUI->CurrentCursor = EditorUI::E_CROSS;
		}

		if (Input::IsRMBDown && !ViewportLock)
		{
			ViewportLock = true;
			InitialMousePosition = Input::MouseLocation;
		}
	}
	if (ViewportLock && !Input::IsRMBDown)
	{
		Application::SetCursorPosition(InitialMousePosition);
		ViewportLock = false;
	}
	if (Input::IsLMBDown && !PressedLMB && !Editor::DraggingTab && !TabInstances[Tabs[SelectedTab].Index])
	{
		PressedLMB = true;
		if (Maths::IsPointIn2DBox(Viewport->Position, Viewport->Position + Viewport->Scale, Input::MouseLocation) && !UI::HoveredButton)
		{
			Vector3 DistanceScaleMultiplier;
			if (SelectedObjects.size() > 0)
				DistanceScaleMultiplier = Vector3((SelectedObjects.at(0)->GetTransform().Location - Vector3::Vec3ToVector(Graphics::MainCamera->Position)).Length() * 0.15f);

			bool Hit = false;
			if (SelectedObjects.size() > 0)
			{
				float t = INFINITY;
				Collision::HitResponse
					CollisionTest = Collision::LineCheckForAABB((ArrowBoxZ * DistanceScaleMultiplier) + SelectedObjects.at(0)->GetTransform().Location,
						Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);
				if (CollisionTest.Hit)
				{
					PreviousLocation = SelectedObjects[0]->GetTransform().Location;
					Hit = true;
					Dragging = true;
					Axis = Vector3(0, 0, 1.f);
					BoxAxis = Vector3(0, 1, 1);
					t = CollisionTest.t;
				}
				CollisionTest = Collision::LineCheckForAABB((ArrowBoxY * DistanceScaleMultiplier) + SelectedObjects.at(0)->GetTransform().Location,
					Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);
				if (CollisionTest.Hit && CollisionTest.t < t)
				{
					PreviousLocation = SelectedObjects[0]->GetTransform().Location;
					Hit = true;
					Dragging = true;
					Axis = Vector3(0, 1.f, 0);
					t = CollisionTest.t;
				}
				CollisionTest = Collision::LineCheckForAABB((ArrowBoxX * DistanceScaleMultiplier) + SelectedObjects.at(0)->GetTransform().Location,
					Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);
				if (CollisionTest.Hit && CollisionTest.t < t)
				{
					PreviousLocation = SelectedObjects[0]->GetTransform().Location;
					Hit = true;
					Dragging = true;
					Axis = Vector3(1.f, 0, 0);
					BoxAxis = Vector3(1, 1, 0);
					t = CollisionTest.t;
				}
				FirstDragFrame = true;
				DragOffset = 0;
				InitialMousePosition = Input::MouseLocation;
			}

			Collision::HitResponse CollisionTest = Collision::LineTrace(Vector3::Vec3ToVector(Graphics::MainCamera->Position),
				(Rotation * 50000.f) + Vector3::Vec3ToVector(Graphics::MainCamera->Position));
			if (CollisionTest.Hit && !Hit)
			{
				if (!Input::IsKeyDown(SDLK_LSHIFT))
				{
					ClearSelectedObjects();
				}
				CollisionTest.HitObject->IsSelected = true;
			}
		}
	}
	if (PressedLMB && !Input::IsLMBDown)
	{
		PressedLMB = false;
		if (Dragging)
		{
			Dragging = false;
			Editor::CurrentUI->UIElements[6]->UpdateLayout();
		}
	}


	if (ViewportLock && !Dragging)
	{
		Input::CursorVisible = false;
		Graphics::MainCamera->OnMouseMoved(Input::MouseMovement.X * 6, -Input::MouseMovement.Y * 6);

		float MovementSpeed = 50;

		if (Input::IsKeyDown(SDLK_LCTRL))
		{
			MovementSpeed *= 0.25;
		}

		if (Input::IsKeyDown(SDLK_LSHIFT))
		{
			MovementSpeed *= 5;
		}

		if (Input::IsKeyDown(SDLK_w))
		{
			Graphics::MainCamera->MoveForward(Performance::DeltaTime * MovementSpeed);
		}
		if (Input::IsKeyDown(SDLK_s))
		{
			Graphics::MainCamera->MoveForward(Performance::DeltaTime * -MovementSpeed);
		}
		if (Input::IsKeyDown(SDLK_d))
		{
			Graphics::MainCamera->MoveRight(Performance::DeltaTime * MovementSpeed);
		}
		if (Input::IsKeyDown(SDLK_a))
		{
			Graphics::MainCamera->MoveRight(Performance::DeltaTime * -MovementSpeed);
		}
		if (Input::IsKeyDown(SDLK_e))
		{
			Graphics::MainCamera->MoveUp(Performance::DeltaTime * MovementSpeed);
		}
		if (Input::IsKeyDown(SDLK_q))
		{
			Graphics::MainCamera->MoveUp(Performance::DeltaTime * -MovementSpeed);
		}
	}
	else
	{
		Input::CursorVisible = true;
	}
	if (Dragging)
	{
		Vector3 DistanceScaleMultiplier;
		if (SelectedObjects.size() > 0)
			DistanceScaleMultiplier = Vector3((SelectedObjects.at(0)->GetTransform().Location - Vector3::Vec3ToVector(Graphics::MainCamera->Position)).Length() * 0.15f);

		Collision::Box TransformBox
		(
			-100000.0f, 100000.0f,
			-100000.0f, 100000.0f,
			-100000.0f, 100000.0f

		);
		TransformBox = TransformBox.TransformBy(Transform(SelectedObjects[0]->GetTransform().Location, 0, BoxAxis));

		auto h = Collision::LineCheckForAABB(TransformBox, Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);

		//if (h.Hit)
		//{
		//	Objects::SpawnObject<MeshObject>(Transform(h.ImpactPoint, 0, 1))->LoadFromFile("Skybox");
		//}
		//return;

		Vector3 TransformToAdd = (h.ImpactPoint - SelectedObjects[0]->GetTransform().Location) * Axis - DragOffset;

		if (FirstDragFrame)
		{
			DragOffset = TransformToAdd;
			TransformToAdd = 0;
			FirstDragFrame = false;
		}
		for (int i = 0; i < Objects::AllObjects.size(); i++)
		{
			if (Objects::AllObjects.at(i)->IsSelected)
			{
				Objects::AllObjects.at(i)->SetTransform(Objects::AllObjects.at(i)->GetTransform() + Transform(TransformToAdd, Vector3(), Vector3(1)));
			}
		}
		ChangedScene = true;
	}

}
void Viewport::UpdateTabBar()
{
	TabBox->SetPosition(Position + Vector2(0, Scale.Y - 0.05));
	TabBox->SetMinSize(Vector2(Scale.X, 0.05));
	TabBox->SetMaxSize(Vector2(Scale.X, 0.05));
	TabBox->DeleteChildren();
	for (size_t i = 0; i < Tabs.size(); i++)
	{
		auto elem = (new UIButton(true, 0, UIColors[0] * (SelectedTab == i ? 2 : 1.5), this, i * 2))
			->SetBorder(UIBox::E_ROUNDED, 0.4)
			->SetPadding(0, 0, 0, 0.02)
			->AddChild((new UIBackground(true, 0, Editor::ItemColors[Tabs[i].Type], Vector2(0.01, 0.05)))
				->SetPadding(0))
			->AddChild((new UIText(0.45, 1, FileUtil::GetFileNameWithoutExtensionFromPath(Tabs[i].Name), Editor::CurrentUI->EngineUIText))
				->SetPadding(0.005, 0.005, 0.005, 0.005));
		elem->SetTryFill(true);
		if (Tabs[i].CanBeClosed)
		{
			elem->AddChild((new UIButton(true, 0, 1, this, i * 2 + 1))
				->SetUseTexture(true, Editor::CurrentUI->Textures[4])
				->SetMinSize(0.04)
				->SetPadding(0.005)
				->SetSizeMode(UIBox::E_PIXEL_RELATIVE));
		}
		TabBox->AddChild(elem);
	}
}

void Viewport::OnButtonClicked(int Index)
{
	if (Index % 2)
	{
		if (TabInstances[Tabs[SelectedTab].Index] && Index / 2 == SelectedTab)
		{
			TabInstances[Tabs[SelectedTab].Index]->Save();
			TabInstances[Tabs[SelectedTab].Index]->TabBackground->IsVisible = false;
			UIBox::RedrawUI();
			SelectedTab--;
		}
		Tabs.erase(Tabs.begin() + Index / 2);
		UpdateLayout();
		return;
	}
	else
	{
		if (TabInstances[Tabs[SelectedTab].Index])
		{
			TabInstances[Tabs[SelectedTab].Index]->Save();
			TabInstances[Tabs[SelectedTab].Index]->TabBackground->IsVisible = false;
			TabInstances[Tabs[SelectedTab].Index]->TabBackground->Update();
			UIBox::RedrawUI();
		}
		SelectedTab = Index / 2;
		if (TabInstances[Tabs[SelectedTab].Index])
		{
			TabInstances[Tabs[SelectedTab].Index]->Load(Tabs[SelectedTab].Name);
		}
		UpdateLayout();
	}
}
void Viewport::OpenTab(size_t TabID, std::string File)
{
	Tabs.push_back(Tab(TabID, File, true, FileUtil::GetExtension(File)));
	SelectedTab = Tabs.size() - 1;
	if (TabInstances[Tabs[SelectedTab].Index])
	{
		TabInstances[Tabs[SelectedTab].Index]->Load(Tabs[SelectedTab].Name);
	}
	UpdateLayout();
}

#endif
