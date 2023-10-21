#ifdef ENGINE_CSHARP
#include "NativeFunctions.h"
#include <CSharp/CSharpInterop.h>
#include <Engine/EngineError.h>
#include <Engine/File/Assets.h>
#include <Rendering/Graphics.h>
#include <Engine/Log.h>
#include <Engine/Scene.h>
#include <Objects/Components/MeshComponent.h>
#include <Objects/Components/CollisionComponent.h>
#include <Objects/Components/CameraComponent.h>
#include <Objects/Components/ParticleComponent.h>
#include <Objects/Components/MoveComponent.h>
#include <Objects/CSharpObject.h>
#include <Rendering/Camera/CameraShake.h>
#include <Engine/Input.h>
#include <Sound/Sound.h>
#include <Math/Collision/Collision.h>
#include <Engine/Console.h>

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

	CameraComponent* NewCameraComponent(float FOV, WorldObject* Parent)
	{
		CameraComponent* NewCamera = new CameraComponent();
		Parent->Attach(NewCamera);
		NewCamera->SetFOV(FOV);
		return NewCamera;
	}

	ParticleComponent* NewParticleComponent(const char* ParticleFile, WorldObject* Parent)
	{
		ParticleComponent* Particle = new ParticleComponent();
		Parent->Attach(Particle);
		Particle->LoadParticle(ParticleFile);
		return Particle;
	}

	MoveComponent* NewMoveComponent(WorldObject* Parent)
	{
		MoveComponent* Movement = new MoveComponent();
		Parent->Attach(Movement);
		return Movement;
	}

	void MovementComponentJump(MoveComponent* Target)
	{
		Target->Jump();
	}

	void MovementComponentAddMovementInput(Vector3 Direction, MoveComponent* Target)
	{
		Target->AddMovementInput(Direction);
	}

	void UseCamera(CameraComponent* Cam)
	{
		Cam->Use();
	}

	void DestroyComponent(Component* c, WorldObject* Parent)
	{
		Parent->Detach(c);
	}
	
	void SetComponentTransform(Component* c, Transform NewTransform)
	{
		c->RelativeTransform = NewTransform;
	}

	Transform GetComponentTransform(Component* c)
	{
		return c->RelativeTransform;
	}

	Vector3 GetMouseMovement()
	{
		return Vector3(Input::MouseMovement, 0);
	}

	void LoadScene(const char* SceneName)
	{
		Scene::LoadNewScene(SceneName);
	}

	CSharp::CSharpWorldObject NewCSObject(const char* TypeName, Transform ObjectTransform)
	{
		CSharpObject* NewObject = Objects::SpawnObject<CSharpObject>(ObjectTransform);
		NewObject->LoadClass(TypeName);
		return NewObject->CS_Obj;
	}

	void DestroyObject(WorldObject* Ptr)
	{
		Objects::DestroyObject(Ptr);
	}

	Sound::SoundBuffer* LoadSound(const char* File)
	{
		return Sound::LoadSound(File);
	}

	void PlaySound(Sound::SoundBuffer* s, float Pitch, float Volume, bool Looping)
	{
		Sound::PlaySound2D(s, Pitch, Volume, Looping);
	}
	
	void UnloadSound(Sound::SoundBuffer* s)
	{
		delete s;
	}

	Collision::HitResponse NativeRaycast(Vector3 Start, Vector3 End, WorldObject* ObjectsToIgnore)
	{
		return Collision::LineTrace(Start, End, {ObjectsToIgnore});
	}

	bool CallConsoleCommand(const char* cmd)
	{
		return Console::ExecuteConsoleCommand(cmd);
	}

	Transform GetObjectTransform(WorldObject* TargetObject)
	{
		return TargetObject->GetTransform();
	}

	void SetObjectTransform(Transform NewTransform, WorldObject* TargetObject)
	{
		TargetObject->SetTransform(NewTransform);
	}
}

#define REGISTER_FUNCTION(func) CSharp::RegisterNativeFunction(# func, func)

void NativeFunctions::RegisterNativeFunctions()
{
	using namespace Input;
	using namespace CameraShake;

	ENGINE_ASSERT(CSharp::IsAssemblyLoaded(), "Assembly should always be loaded first before registering any native functions.");

	REGISTER_FUNCTION(NewMeshComponent);
	REGISTER_FUNCTION(NewCollisionComponent);
	REGISTER_FUNCTION(NewCameraComponent);
	REGISTER_FUNCTION(NewParticleComponent);
	REGISTER_FUNCTION(NewMoveComponent);

	REGISTER_FUNCTION(GetObjectTransform);
	REGISTER_FUNCTION(SetObjectTransform);
	REGISTER_FUNCTION(DestroyObject);

	REGISTER_FUNCTION(DestroyComponent);
	REGISTER_FUNCTION(SetComponentTransform);
	REGISTER_FUNCTION(GetComponentTransform);

	REGISTER_FUNCTION(UseCamera);
	REGISTER_FUNCTION(MovementComponentAddMovementInput);
	REGISTER_FUNCTION(MovementComponentJump);

	REGISTER_FUNCTION(IsKeyDown);
	REGISTER_FUNCTION(GetMouseMovement);
	REGISTER_FUNCTION(PlayDefaultCameraShake);
	REGISTER_FUNCTION(NewCSObject);
	REGISTER_FUNCTION(LoadScene);
	REGISTER_FUNCTION(LoadSound);
	REGISTER_FUNCTION(PlaySound);
	REGISTER_FUNCTION(UnloadSound);
	REGISTER_FUNCTION(NativeRaycast);
	REGISTER_FUNCTION(CallConsoleCommand);
	REGISTER_FUNCTION(Vector3::GetScaledAxis);
}

#endif