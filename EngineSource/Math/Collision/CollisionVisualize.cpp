#if !SERVER
#include "CollisionVisualize.h"
#include <vector>
#include <Rendering/Mesh/Model.h>
#include <Objects/WorldObject.h>
#include <Objects/Components/CollisionComponent.h>
#include <Engine/Application.h>
#include <Math/Physics/Physics.h>
#include <Rendering/Graphics.h>
#include <Rendering/Utility/Framebuffer.h>
#include <Engine/Log.h>
#include <Objects/Components/PhysicsComponent.h>

namespace CollisionVisualize
{
	FramebufferObject* VisualizeFramebuffer = nullptr;

	struct VisualizeModel
	{
		Model* ModelObj = nullptr;
		Physics::PhysicsBody* Body;
		bool Dynamic = false;
	};
	std::vector<VisualizeModel> Models;

	size_t LastObjectsSize = 0;
}

void CollisionVisualize::Activate()
{
	Graphics::MainFramebuffer->Active = false;
	if (VisualizeFramebuffer)
	{
		VisualizeFramebuffer->ClearContent(true);
		delete VisualizeFramebuffer;
	}
	Models.clear();

	VisualizeFramebuffer = new FramebufferObject();

#if !RELEASE
	VisualizeFramebuffer->AddEditorGrid();
#endif

	std::string MaterialPath = Application::GetEditorPath() + "/EditorContent/Materials/CollisionVisualize/";

	for (WorldObject* Obj : Objects::AllObjects)
	{
		for (Component* c : Obj->GetComponents())
		{
			ModelGenerator::ModelData MeshData;
			Transform MeshTransform;
			Physics::PhysicsBody* Body = nullptr;

			CollisionComponent* StaticCollider = dynamic_cast<CollisionComponent*>(c);
			if (StaticCollider)
			{
				Body = static_cast<Physics::PhysicsBody*>(StaticCollider->Collider);


				if (Body->Type == Physics::PhysicsBody::BodyType::Mesh)
				{
					MeshData = static_cast<Physics::MeshBody*>(Body)->MeshData;
				}

				for (auto& i : MeshData.Elements)
				{
					i.ElemMaterial = MaterialPath + "Collider.jsmat";
				}

				MeshData.TwoSided = true;
				MeshData.CastShadow = false;
				MeshTransform = Transform(Body->GetPosition(), Body->GetRotation().DegreesToRadians(), Body->BodyTransform.Scale);

			}
			PhysicsComponent* DynamicCollider = dynamic_cast<PhysicsComponent*>(c);
			if (DynamicCollider)
			{
#if RELEASE
				continue;
#endif

				Body = static_cast<Physics::PhysicsBody*>(DynamicCollider->PhysicsBodyPtr);

				MeshTransform = Transform(Body->GetPosition(), Body->GetRotation().DegreesToRadians(), Body->BodyTransform.Scale);

				switch (Body->Type)
				{
				case Physics::PhysicsBody::BodyType::Box:
					MeshData.AddElement().MakeCube(2, 0);
					MeshTransform.Scale = MeshTransform.Scale / 2.5f;
					break;
				case Physics::PhysicsBody::BodyType::Sphere:
					MeshData.LoadModelFromFile(Application::GetEditorPath() + "/EditorContent/Models/Sphere.jsm");
					MeshTransform.Scale = MeshTransform.Scale / 2.5f;
					break;
				case Physics::PhysicsBody::BodyType::Capsule:
					MeshData.LoadModelFromFile(Application::GetEditorPath() + "/EditorContent/Models/Sphere.jsm");
					{
						float diff = (MeshTransform.Scale.Y - MeshTransform.Scale.X) * 45;

						for (auto& i : MeshData.Elements[0].Vertices)
						{
							if (i.Position.y > 0)
							{
								i.Position.y += diff;
							}
							else
							{
								i.Position.y -= diff;
							}
						}
					}
					MeshTransform.Scale = MeshTransform.Scale.X / 3;
					break;
				default:
					break;
				}

				MeshData.TwoSided = false;
				MeshData.CastShadow = false;

				std::string ColliderMaterial = MaterialPath;

				switch (Body->ColliderMovability)
				{
				case Physics::MotionType::Dynamic:
					ColliderMaterial.append("Dynamic");
					break;
				case Physics::MotionType::Kinematic:
					ColliderMaterial.append("Kinematic");
					break;
				case Physics::MotionType::Static:
					ColliderMaterial.append("Static");
					break;
				default:
					break;
				}
				ColliderMaterial.append("PhysicsBody.jsmat");

				for (auto& i : MeshData.Elements)
				{
					i.ElemMaterial = ColliderMaterial;
				}
			}

			if (Body)
			{
				Model* StaticModel = new Model(MeshData);
				StaticModel->ModelTransform = MeshTransform;
				StaticModel->UpdateTransform();
				VisualizeFramebuffer->FramebufferCamera = Graphics::MainFramebuffer->FramebufferCamera;
				VisualizeFramebuffer->Renderables.push_back(StaticModel);
				VisualizeModel m =
				{
					.ModelObj = StaticModel,
					.Body = Body,
					.Dynamic = (bool)DynamicCollider
				};
				Models.push_back(m);
			}
		}
	}
	LastObjectsSize = Objects::AllObjects.size();
}

void CollisionVisualize::Deactivate()
{
	Graphics::MainFramebuffer->Active = true;
	Models.clear();
	if (VisualizeFramebuffer)
	{
		VisualizeFramebuffer->ClearContent(true);
		delete VisualizeFramebuffer;
		VisualizeFramebuffer = nullptr;
	}
}

void CollisionVisualize::Update()
{
	if (!GetIsActive())
	{
		return;
	}

	if (LastObjectsSize != Objects::AllObjects.size())
	{
		Activate();
		return;
	}

	for (auto& i : Models)
	{
		Transform New = Transform(i.Body->GetPosition(), -i.Body->GetRotation().DegreesToRadians(), i.Body->BodyTransform.Scale);
		if (i.Dynamic)
		{
			New.Scale = i.ModelObj->ModelTransform.Scale;
		}

		if (New != i.ModelObj->ModelTransform)
		{
			i.ModelObj->ModelTransform = New;
			i.ModelObj->UpdateTransform();
		}
	}

}

FramebufferObject* CollisionVisualize::GetVisualizeBuffer()
{
	return VisualizeFramebuffer;
}
bool CollisionVisualize::GetIsActive()
{
	return VisualizeFramebuffer;
}
#endif