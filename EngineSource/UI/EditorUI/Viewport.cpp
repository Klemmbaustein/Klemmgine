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

Viewport::Viewport(Vector3* Colors, Vector2 Position, Vector2 Scale) : EditorTab(Colors, Position, Scale, 0, 2)
{
	OutlineBuffer = new FramebufferObject();
	OutlineBuffer->FramebufferCamera = Graphics::MainCamera;

	TabBackground->SetOpacity(0);
}

void Viewport::ClearSelectedObjects()
{
	for (auto i : SelectedObjects)
	{
		i->IsSelected = false;
	}
	SelectedObjects.clear();
}

void Viewport::Save()
{
}

void Viewport::Load(std::string File)
{
}
void Viewport::UpdateLayout()
{
}
void Viewport::Tick()
{
	UpdateTab();

	OutlineBuffer->Renderables.clear();
	OutlineBuffer->FramebufferCamera = Graphics::MainCamera;

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

	if (Input::IsKeyDown(SDLK_ESCAPE))
	{
		ClearSelectedObjects();
	}

	auto Viewport = Editor::CurrentUI->UIElements[4];

	if (Maths::IsPointIn2DBox(Viewport->Position, Viewport->Position + Viewport->Scale, Input::MouseLocation))
	{
		if (Input::IsLMBDown && !PressedLMB && !Editor::DraggingTab)
		{
			PressedLMB = true;
			Vector2 RelativeMouseLocation = Input::MouseLocation - (Viewport->Position + (Viewport->Scale * 0.5));
			Vector3 Direction = Graphics::MainCamera->ForwardVectorFromScreenPosition(RelativeMouseLocation.X, RelativeMouseLocation.Y);
			
			auto hit = Collision::LineTrace(Graphics::MainCamera->Position, Graphics::MainCamera->Position + Direction * 100);
			if (!Input::IsKeyDown(SDLK_LSHIFT))
			{
				ClearSelectedObjects();
			}

			if (hit.Hit)
			{
				hit.HitObject->IsSelected = true;
			}
		}

		if (!Editor::CurrentUI->CurrentCursor) // Default Cursor = 0. So if the current cursor evaluates to 'false' its the default cursor
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
	if (Input::IsLMBDown && !PressedLMB)
	{
		PressedLMB = true;

	}
	if (PressedLMB && !Input::IsLMBDown)
	{
		PressedLMB = false;
	}

	if (ViewportLock)
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

}
#endif
