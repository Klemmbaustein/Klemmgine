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
#include <UI/UIBox.h>
#include <UI/UIBackground.h>
#include <UI/UIText.h>
#include <Engine/Application.h>

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
		CSharpObject* NewObject = Objects::SpawnObject<CSharpObject>(ObjectTransform, 0);
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

#if !SERVER
	UIBox* CreateUIBox(bool Horizontal, Vector2 Position)
	{
#if !EDITOR
		return new UIBox(Horizontal, Position);
#endif
		return nullptr;
	}

	void DestroyUIBox(UIBox* Target)
	{
#if !EDITOR
		delete Target;
#endif
	}

	void SetUIBoxMinSize(Vector2 NewMinSize, UIBox* Target)
	{
#if !EDITOR
		Target->SetMinSize(NewMinSize);
#endif
	}

	void SetUIBoxMaxSize(Vector2 NewMaxSize, UIBox* Target)
	{
#if !EDITOR
		Target->SetMaxSize(NewMaxSize);
#endif
	}

	void SetUIBoxPosition(Vector2 Position, UIBox* Target)
	{
#if !EDITOR
		Target->SetPosition(Position);
#endif
	}

	void SetUIBoxVerticalAlign(UIBox::Align NewAlign, UIBox* Target)
	{
#if !EDITOR
		Target->SetVerticalAlign(NewAlign);
#endif
	}

	void SetUIBoxHorizontalAlign(UIBox::Align NewAlign, UIBox* Target)
	{
#if !EDITOR
		Target->SetHorizontalAlign(NewAlign);
#endif
	}

	void SetUIBoxSizeMode(UIBox::SizeMode Mode, UIBox* Target)
	{
#if !EDITOR
		Target->SetSizeMode(Mode);
#endif
	}

	void SetUIBoxBorder(UIBox::BorderType NewBorder, float Size, UIBox* Target)
	{
#if !EDITOR
		Target->SetBorder(NewBorder, Size);
#endif
	}

	void SetUIBoxPadding(float Up, float Down, float Left, float Right, UIBox* Target)
	{
#if !EDITOR
		Target->SetPadding(Up, Down, Left, Right);
#endif
	}

	void SetUIBoxPaddingSizeMode(UIBox::SizeMode Mode, UIBox* Target)
	{
#if !EDITOR
		Target->SetPaddingSizeMode(Mode);
#endif
	}

	void AddUIBoxChild(UIBox* Child, UIBox* Target)
	{
#if !EDITOR
		Target->AddChild(Child);
#endif
	}


	UIBackground* CreateUIBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale)
	{
#if EDITOR
		return nullptr;
#endif
		return (UIBackground*)(new UIBackground(Horizontal, Position, Color, MinScale))->SetTryFill(true);
	}

	TextRenderer* CreateTextRenderer(const char* Font)
	{
#if EDITOR
		return nullptr;
#endif
		return new TextRenderer(Font);
	}

	UIText* CreateUIText(float Scale, Vector3 Color, const char* Text, TextRenderer* Renderer)
	{
#if EDITOR
		return nullptr;
#endif
		return new UIText(Scale, Color, Text, Renderer);
	}

	void SetCursorVisible(bool NewVisible)
	{
#if !EDITOR
		Input::CursorVisible = NewVisible;
#endif
	}
#endif
}

#define REGISTER_FUNCTION(func) CSharp::RegisterNativeFunction(# func, func)

void NativeFunctions::RegisterNativeFunctions()
{
	using namespace Input;
	using namespace CameraShake;
	using namespace Application;

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

#if !SERVER
	REGISTER_FUNCTION(SetCursorVisible);

	REGISTER_FUNCTION(CreateUIBox);
	REGISTER_FUNCTION(DestroyUIBox);
	REGISTER_FUNCTION(SetUIBoxMinSize);
	REGISTER_FUNCTION(SetUIBoxMaxSize);
	REGISTER_FUNCTION(AddUIBoxChild);
	REGISTER_FUNCTION(SetUIBoxHorizontalAlign);
	REGISTER_FUNCTION(SetUIBoxVerticalAlign);
	REGISTER_FUNCTION(SetUIBoxSizeMode);
	REGISTER_FUNCTION(SetUIBoxBorder);
	REGISTER_FUNCTION(SetUIBoxPadding);
	REGISTER_FUNCTION(SetUIBoxPosition);
	REGISTER_FUNCTION(SetUIBoxPaddingSizeMode);

	REGISTER_FUNCTION(CreateUIBackground);

	REGISTER_FUNCTION(CreateUIText);
	REGISTER_FUNCTION(CreateTextRenderer);
#endif

}

#endif