#if EDITOR
#include "Viewport.h"
#include <Engine/Input.h>
#include <UI/EditorUI/EditorUI.h>
#include <Math/Math.h>
#include <Engine/Application.h>
#include <Objects/Components/BillboardComponent.h>
#include <Objects/Components/MeshComponent.h>
#include <Objects/Components/InstancedMeshComponent.h>
#include <Rendering/Mesh/Model.h>
#include <Rendering/Mesh/InstancedModel.h>
#include <Rendering/BillboardSprite.h>
#include <Math/Collision/Collision.h>
#include <Engine/Log.h>
#include <Rendering/Mesh/Model.h>
#include <UI/EditorUI/Tabs/MeshTab.h>
#include <UI/EditorUI/Tabs/MaterialTab.h>
#include <UI/EditorUI/Tabs/ParticleEditorTab.h>
#include <UI/EditorUI/Tabs/CubemapTab.h>
#include <Engine/Utility/FileUtility.h>
#include <Objects/MeshObject.h>
#include <Objects/CSharpObject.h>
#include "ContextMenu.h"
#include <Objects/ParticleObject.h>
#include <Objects/SoundObject.h>

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

UIBackground* TestCursor = nullptr;

Viewport::Viewport(EditorPanel* Parent) : EditorPanel(Parent, "Viewport")
{
	ViewportInstance = this;

	OutlineBuffer = new FramebufferObject();
	OutlineBuffer->FramebufferCamera = Graphics::MainCamera;
	ArrowsBuffer = new FramebufferObject();
	ArrowsBuffer->FramebufferCamera = Graphics::MainCamera;
	ArrowsModel = new Model(Application::GetEditorPath() + "/EditorContent/Models/Arrows.jsm");
	ArrowsModel->ModelTransform.Scale = Vector3(1, 1, 1);
	ArrowsModel->ModelTransform.Rotation.Y = -Math::PI_F / 2.0f;
	ArrowsBuffer->Renderables.push_back(ArrowsModel);
	BackgroundVisible = false;
}

void Viewport::ClearSelectedObjects()
{
	for (auto i : EditorUI::SelectedObjects)
	{
		i->IsSelected = false;
	}
	EditorUI::SelectedObjects.clear();
}

void Viewport::OnItemDropped(DroppedItem Item)
{
	Vector2 RelativeMouseLocation = Application::GetCursorPosition() - (Position + (Scale * 0.5));
	Vector3 Direction = Graphics::MainCamera->ForwardVectorFromScreenPosition(RelativeMouseLocation.X, RelativeMouseLocation.Y);

	Vector3 Point = (Direction * 100.0f) + Graphics::MainCamera->Position;

	auto hit = Collision::LineTrace(Graphics::MainCamera->Position, Point);

	if (hit.Hit)
	{
		Point = hit.ImpactPoint;
	}

	if (Item.TypeID == CSharpObject::GetID())
	{
		auto Obj = Objects::SpawnObject<CSharpObject>(Transform(Point, 0, 1));
		if (Item.Path != "CSharpObject")
		{
			Obj->LoadClass(Item.Path);
		}
		Obj->IsSelected = true;
		return;
	}
	else if (!std::filesystem::exists(Item.Path))
	{
		Objects::SpawnObjectFromID(Item.TypeID, Transform(Point, 0, 1))->IsSelected = true;
		return;
	}

	ClearSelectedObjects();
	std::string Ext = FileUtil::GetExtension(Item.Path);

	if (Ext == "jsm")
	{
		auto Obj = Objects::SpawnObject<MeshObject>(Transform(Point, 0, 1));
		Obj->LoadFromFile(FileUtil::GetFileNameWithoutExtensionFromPath(Item.Path));
		Obj->Name = FileUtil::GetFileNameWithoutExtensionFromPath(Item.Path);
		Obj->IsSelected = true;
	}

	if (Ext == "jspart")
	{
		auto Obj = Objects::SpawnObject<ParticleObject>(Transform(Point, 0, 1));
		Obj->LoadParticle(FileUtil::GetFileNameWithoutExtensionFromPath(Item.Path));
		Obj->IsSelected = true;
	}

	if (Ext == "jscn")
	{
		EditorUI::OpenScene(Item.Path);
	}

	if (Ext == "wav")
	{
		auto Obj = Objects::SpawnObject<SoundObject>(Transform(Point, 0, 1));
		Obj->LoadSound(FileUtil::GetFileNameWithoutExtensionFromPath(Item.Path));
		Obj->IsSelected = true;
	}
}

void Viewport::OnResized()
{
}

void Viewport::Tick()
{
	TickPanel();

	if (CurrentMainBuffer != Graphics::MainFramebuffer)
	{
		Graphics::MainFramebuffer->AddEditorGrid();
		
		CurrentMainBuffer = Graphics::MainFramebuffer;
	}

	Graphics::MainCamera->FOV = Math::PI_F / 1.2f;

	if (EditorPanel::Dragged)
	{
		return;
	}

	OutlineBuffer->Renderables.clear();
	OutlineBuffer->FramebufferCamera = Graphics::MainCamera;
	ArrowsBuffer->FramebufferCamera = Graphics::MainCamera;

	ArrowsModel->Visible = EditorUI::SelectedObjects.size();
	if (EditorUI::SelectedObjects.size())
	{
		ArrowsModel->ModelTransform.Position = EditorUI::SelectedObjects[0]->GetTransform().Position;
		ArrowsModel->ModelTransform.Scale = Vector3(1, 1, -1) * Vector3::Distance(Graphics::MainCamera->Position, ArrowsModel->ModelTransform.Position) * 0.03f;
		ArrowsModel->UpdateTransform();
	}

	EditorUI::SelectedObjects.clear();
	for (auto i : Objects::AllObjects)
	{
		if (!i->IsSelected) continue;
		EditorUI::SelectedObjects.push_back(i);

		for (auto j : i->GetComponents())
		{
			if (dynamic_cast<MeshComponent*>(j))
			{
				Model* Mdl = dynamic_cast<MeshComponent*>(j)->GetModel();
				if (Mdl == nullptr)
				{
					continue;
				}
				OutlineBuffer->Renderables.push_back(Mdl);
			}
			if (dynamic_cast<InstancedMeshComponent*>(j))
			{
				if (dynamic_cast<InstancedMeshComponent*>(j)->GetInstancedModel())
				{
					OutlineBuffer->Renderables.push_back(dynamic_cast<InstancedMeshComponent*>(j)->GetInstancedModel());
				}
			}
			if (dynamic_cast<BillboardComponent*>(j))
			{
				OutlineBuffer->Renderables.push_back(dynamic_cast<BillboardComponent*>(j)->GetSprite());
			}
		}
	}
	if (PreviousSelectedObject && !EditorUI::SelectedObjects.size())
	{
		PreviousSelectedObject = nullptr;
		EditorUI::OnObjectSelected();
	}
	else if (EditorUI::SelectedObjects.size() && PreviousSelectedObject != EditorUI::SelectedObjects[0])
	{
		PreviousSelectedObject = EditorUI::SelectedObjects[0];
		EditorUI::OnObjectSelected();
	}
	else if (PreviousSelectedObjectSize != EditorUI::SelectedObjects.size())
	{
		PreviousSelectedObjectSize = EditorUI::SelectedObjects.size();
		EditorUI::OnObjectSelected();
	}

	if (Input::IsKeyDown(Input::Key::ESCAPE))
	{
		ClearSelectedObjects();
	}

	if (Input::IsKeyDown(Input::Key::DELETE) && !TextInput::PollForText)
	{
		for (int i = 0; i < Objects::AllObjects.size(); i++)
		{
			if (Objects::AllObjects.at(i)->IsSelected)
			{
				Objects::DestroyObject(Objects::AllObjects[i]);
				ChangedScene = true;
			}
		}
	}

	SetName(ChangedScene ? "Viewport*" : "Viewport");

	Vector2 RelativeMouseLocation = Application::GetCursorPosition() - (Position + (Scale * 0.5));
	Vector3 Rotation = Graphics::MainCamera->ForwardVectorFromScreenPosition(RelativeMouseLocation.X, RelativeMouseLocation.Y);

	if (UI::HoveredBox == PanelMainBackground
		&& !Dragging)
	{
		// Default Cursor = 0. So if the current cursor evaluates to 'false' its the default cursor
		if (!(int)Application::EditorInstance->CurrentCursor)
		{
			Application::EditorInstance->CurrentCursor = EditorUI::CursorType::Cross;
		}

		if (Input::IsRMBDown && !ViewportLock)
		{
			ViewportLock = true;
			InitialMousePosition = Input::MouseLocation;
		}
	}

	if (!ViewportLock && Input::IsKeyDown(Input::Key::LCTRL) && Input::IsKeyDown(Input::Key::d))
	{
		if (!IsCopying)
		{
			IsCopying = true;
			std::vector<WorldObject*> CopiedObjects;
			for (WorldObject* i : EditorUI::SelectedObjects)
			{
				WorldObject* o = Objects::SpawnObjectFromID(i->GetObjectDescription().ID, i->GetTransform());
				o->Name = i->Name;
				o->Deserialize(i->Serialize());
				o->LoadProperties(i->GetPropertiesAsString());
				o->OnPropertySet();
				o->IsSelected = true;
				CopiedObjects.push_back(o);
			}
			ClearSelectedObjects();
			EditorUI::SelectedObjects = CopiedObjects;
			ChangedScene = true;
		}
	}
	else
	{
		IsCopying = false;
	}
	if (ViewportLock && !Input::IsRMBDown)
	{
		Application::SetCursorPosition(InitialMousePosition);
		ViewportLock = false;
	}
	if (Input::IsLMBDown && !PressedLMB)
	{
		PressedLMB = true;
		if (Math::IsPointIn2DBox(Position, Position + Scale, Input::MouseLocation) && UI::HoveredBox == PanelMainBackground)
		{
			Vector3 DistanceScaleMultiplier;
			if (EditorUI::SelectedObjects.size() > 0)
				DistanceScaleMultiplier = Vector3((EditorUI::SelectedObjects.at(0)->GetTransform().Position 
					- Vector3(Graphics::MainCamera->Position)).Length() * 0.15f);

			bool Hit = false;
			if (EditorUI::SelectedObjects.size() > 0)
			{
				float t = INFINITY;
				Collision::HitResponse CollisionTest 
					= Collision::LineCheckForAABB((ArrowBoxZ * DistanceScaleMultiplier) + EditorUI::SelectedObjects.at(0)->GetTransform().Position,
						Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);
				if (CollisionTest.Hit)
				{
					PreviousLocation = EditorUI::SelectedObjects[0]->GetTransform().Position;
					Hit = true;
					Dragging = true;
					Axis = Vector3(0, 0, 1.0f);
					BoxAxis = 2;
					t = CollisionTest.Distance;
				}
				CollisionTest = Collision::LineCheckForAABB((ArrowBoxY * DistanceScaleMultiplier) + EditorUI::SelectedObjects.at(0)->GetTransform().Position,
					Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);
				if (CollisionTest.Hit && CollisionTest.Distance < t)
				{
					PreviousLocation = EditorUI::SelectedObjects[0]->GetTransform().Position;
					Hit = true;
					Dragging = true;
					BoxAxis = 1;
					Axis = Vector3(0, 1.0f, 0);
					t = CollisionTest.Distance;
				}
				CollisionTest = Collision::LineCheckForAABB((ArrowBoxX * DistanceScaleMultiplier) + EditorUI::SelectedObjects.at(0)->GetTransform().Position,
					Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);
				if (CollisionTest.Hit && CollisionTest.Distance < t)
				{
					PreviousLocation = EditorUI::SelectedObjects[0]->GetTransform().Position;
					Hit = true;
					Dragging = true;
					Axis = Vector3(1.0f, 0, 0);
					BoxAxis = 0;
					t = CollisionTest.Distance;
				}
				FirstDragFrame = true;
				DragOffset = 0;
				InitialMousePosition = Input::MouseLocation;
			}

			Collision::HitResponse CollisionTest = Collision::LineTrace(
				Graphics::MainCamera->Position,
				(Rotation * 50000.f) + Graphics::MainCamera->Position);
			if (CollisionTest.Hit && !Hit)
			{
				if (!Input::IsKeyDown(Input::Key::LSHIFT))
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
			EditorUI::UpdateAllInstancesOf<ContextMenu>();
			Dragging = false;
		}
	}


	if (ViewportLock && !Dragging)
	{
		Input::CursorVisible = false;
		Graphics::MainCamera->OnMouseMoved(Input::MouseMovement.X * 6, -Input::MouseMovement.Y * 6);

		float MovementSpeed = 50;

		if (Input::IsKeyDown(Input::Key::LCTRL))
		{
			MovementSpeed *= 0.25;
		}

		if (Input::IsKeyDown(Input::Key::LSHIFT))
		{
			MovementSpeed *= 5;
		}

		if (Input::IsKeyDown(Input::Key::w))
		{
			Graphics::MainCamera->MoveForward(Performance::DeltaTime * MovementSpeed);
		}
		if (Input::IsKeyDown(Input::Key::s))
		{
			Graphics::MainCamera->MoveForward(Performance::DeltaTime * -MovementSpeed);
		}
		if (Input::IsKeyDown(Input::Key::d))
		{
			Graphics::MainCamera->MoveRight(Performance::DeltaTime * MovementSpeed);
		}
		if (Input::IsKeyDown(Input::Key::a))
		{
			Graphics::MainCamera->MoveRight(Performance::DeltaTime * -MovementSpeed);
		}
		if (Input::IsKeyDown(Input::Key::e))
		{
			Graphics::MainCamera->MoveUp(Performance::DeltaTime * MovementSpeed);
		}
		if (Input::IsKeyDown(Input::Key::q))
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
		if (EditorUI::SelectedObjects.size() > 0)
			DistanceScaleMultiplier = Vector3((EditorUI::SelectedObjects.at(0)->GetTransform().Position 
				- Vector3(Graphics::MainCamera->Position)).Length() * 0.15f);

		Collision::Box TransformBox
		(
			-100000.0f, 100000.0f,
			-100000.0f, 100000.0f,
			-100000.0f, 100000.0f
		);
		Vector3 BoxScale = 1;
		int Prev = -1;
		Vector3 Forward = Vector3::GetForwardVector(Graphics::MainCamera->Rotation);
		for (int i = 0; i < 3; i++)
		{
			if (i != BoxAxis)
			{
				if (Prev == -1)
				{
					Prev = i;
				}
				else
				{
					BoxScale.at(i) = std::abs(Forward.at(Prev)) > std::abs(Forward.at(i)) ? 1.0f : 0.0f;
					BoxScale.at(Prev) = std::abs(Forward.at(Prev)) > std::abs(Forward.at(i)) ? 0.0f : 1.0f;
				}
			}
		}

		TransformBox = TransformBox.TransformBy(Transform(EditorUI::SelectedObjects[0]->GetTransform().Position, 0, BoxScale));

		auto h = Collision::LineCheckForAABB(TransformBox, Graphics::MainCamera->Position, (Rotation * 500.f) + Graphics::MainCamera->Position);

		Vector3 TransformToAdd = (h.ImpactPoint - EditorUI::SelectedObjects[0]->GetTransform().Position) * Axis - DragOffset;

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

void Viewport::OnButtonClicked(int Index)
{
}
#endif
