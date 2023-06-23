#ifdef ENGINE_CSHARP
#include "NativeFunctions.h"
#include <CSharp/CSharpInterop.h>
#include <Engine/EngineError.h>
#include <Rendering/Utility/Framebuffer.h>
#include <World/Assets.h>
#include <World/Graphics.h>
#include <Engine/Log.h>
#include <Objects/Components/MeshComponent.h>
#include <Objects/Components/CollisionComponent.h>
#include <Engine/Input.h>

namespace NativeFunctions
{
	MeshComponent* NewMeshComponent(const char* ModelFile, WorldObject* Parent)
	{
		MeshComponent* NewModel = new MeshComponent();
		Parent->Attach(NewModel);
		NewModel->Load(ModelFile);

		return NewModel;
	}

	CollisionComponent* NewCollisionComponent(const char* ModelFile, WorldObject* Parent)
	{
		CollisionComponent* NewCollider = new CollisionComponent();
		Parent->Attach(NewCollider);
		ModelGenerator::ModelData m;
		m.LoadModelFromFile(ModelFile);
		NewCollider->Init(m.GetMergedVertices(), m.GetMergedIndices());
		return NewCollider;
	}

	void DestroyComponent(Component* c, WorldObject* Parent)
	{
		Parent->Detach(c);
	}
}

void NativeFunctions::RegisterNativeFunctions()
{
	ENGINE_ASSERT(CSharp::IsAssemblyLoaded(), "Assembly should always be loaded first before registering any native functions.");
	CSharp::RegisterNativeFunction("NewMeshComponent", NewMeshComponent);
	CSharp::RegisterNativeFunction("NewCollisionComponent", NewCollisionComponent);
	CSharp::RegisterNativeFunction("DestroyComponent", DestroyComponent);
	CSharp::RegisterNativeFunction("IsKeyDown", Input::IsKeyDown);
}

#endif