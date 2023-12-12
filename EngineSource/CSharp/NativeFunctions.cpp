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
	static MeshComponent* NewMeshComponent(const char* ModelFile, WorldObject* Parent)
	{
		MeshComponent* NewModel = new MeshComponent();
		Parent->Attach(NewModel);
		NewModel->Load(ModelFile);

		return NewModel;
	}

	static CollisionComponent* NewCollisionComponent(const char* ModelFile, WorldObject* Parent)
	{
		CollisionComponent* NewCollider = new CollisionComponent();
		Parent->Attach(NewCollider);
		ModelGenerator::ModelData m;
		m.LoadModelFromFile(ModelFile);
		NewCollider->Init(m.GetMergedVertices(), m.GetMergedIndices());
		return NewCollider;
	}

	static CameraComponent* NewCameraComponent(float FOV, WorldObject* Parent)
	{
		CameraComponent* NewCamera = new CameraComponent();
		Parent->Attach(NewCamera);
		NewCamera->SetFOV(FOV);
		return NewCamera;
	}

	static ParticleComponent* NewParticleComponent(const char* ParticleFile, WorldObject* Parent)
	{
		ParticleComponent* Particle = new ParticleComponent();
		Parent->Attach(Particle);
		Particle->LoadParticle(ParticleFile);
		return Particle;
	}

	static MoveComponent* NewMoveComponent(WorldObject* Parent)
	{
		MoveComponent* Movement = new MoveComponent();
		Parent->Attach(Movement);
		return Movement;
	}

	static void MovementComponentJump(MoveComponent* Target)
	{
		Target->Jump();
	}

	static void MovementComponentAddMovementInput(Vector3 Direction, MoveComponent* Target)
	{
		Target->AddMovementInput(Direction);
	}

	static void UseCamera(CameraComponent* Cam)
	{
		Cam->Use();
	}

	static Collision::HitResponse CollisionComponentOverlap(CollisionComponent** IgnoredComponents, int32_t IgnoredLength, CollisionComponent* Target)
	{
		std::set<CollisionComponent*> Ignored;
		for (int32_t i = 0; i < IgnoredLength; i++)
		{
			Ignored.insert(IgnoredComponents[i]);
		}
		return Target->OverlapCheck(Ignored);
	}

	static void DestroyComponent(Component* c, WorldObject* Parent)
	{
		Parent->Detach(c);
	}
	
	static void SetComponentTransform(Component* c, Transform NewTransform)
	{
		c->RelativeTransform = NewTransform;
	}

	static Transform GetComponentTransform(Component* c)
	{
		return c->RelativeTransform;
	}

	static Vector3 GetMouseMovement()
	{
		return Vector3(Input::MouseMovement, 0);
	}

	static void LoadScene(const char* SceneName)
	{
		Scene::LoadNewScene(SceneName);
	}

	static CSharp::CSharpWorldObject NewCSObject(const char* TypeName, Transform ObjectTransform)
	{
		CSharpObject* NewObject = Objects::SpawnObject<CSharpObject>(ObjectTransform, 0);
		NewObject->LoadClass(TypeName);
		return NewObject->CS_Obj;
	}

	static void DestroyObject(WorldObject* Ptr)
	{
		Objects::DestroyObject(Ptr);
	}

	static Sound::SoundBuffer* LoadSound(const char* File)
	{
		return Sound::LoadSound(File);
	}

	static void PlaySound(Sound::SoundBuffer* s, float Pitch, float Volume, bool Looping)
	{
		Sound::PlaySound2D(s, Pitch, Volume, Looping);
	}
	
	static void UnloadSound(Sound::SoundBuffer* s)
	{
		delete s;
	}

	static Collision::HitResponse NativeRaycast(Vector3 Start, Vector3 End, WorldObject* ObjectsToIgnore)
	{
		return Collision::LineTrace(Start, End, {ObjectsToIgnore});
	}

	static bool CallConsoleCommand(const char* cmd)
	{
		return Console::ExecuteConsoleCommand(cmd);
	}

	static Transform GetObjectTransform(WorldObject* TargetObject)
	{
		return TargetObject->GetTransform();
	}

	static void SetObjectTransform(Transform NewTransform, WorldObject* TargetObject)
	{
		TargetObject->SetTransform(NewTransform);
	}

#if !SERVER
	static UIBox* CreateUIBox(bool Horizontal, Vector2 Position)
	{
#if !EDITOR
		return new UIBox(Horizontal, Position);
#endif
		return nullptr;
	}

	static void DestroyUIBox(UIBox* Target)
	{
#if !EDITOR
		delete Target;
#endif
	}

	static void SetUIBoxMinSize(Vector2 NewMinSize, UIBox* Target)
	{
#if !EDITOR
		Target->SetMinSize(NewMinSize);
#endif
	}

	static void SetUIBoxMaxSize(Vector2 NewMaxSize, UIBox* Target)
	{
#if !EDITOR
		Target->SetMaxSize(NewMaxSize);
#endif
	}

	static void SetUIBoxPosition(Vector2 Position, UIBox* Target)
	{
#if !EDITOR
		Target->SetPosition(Position);
#endif
	}

	static void SetUIBoxVerticalAlign(UIBox::Align NewAlign, UIBox* Target)
	{
#if !EDITOR
		Target->SetVerticalAlign(NewAlign);
#endif
	}

	static void SetUIBoxHorizontalAlign(UIBox::Align NewAlign, UIBox* Target)
	{
#if !EDITOR
		Target->SetHorizontalAlign(NewAlign);
#endif
	}

	static void SetUIBoxSizeMode(UIBox::SizeMode Mode, UIBox* Target)
	{
#if !EDITOR
		Target->SetSizeMode(Mode);
#endif
	}

	static void SetUIBoxBorder(UIBox::BorderType NewBorder, float Size, UIBox* Target)
	{
#if !EDITOR
		Target->SetBorder(NewBorder, Size);
#endif
	}

	static void SetUIBoxPadding(float Up, float Down, float Left, float Right, UIBox* Target)
	{
#if !EDITOR
		Target->SetPadding(Up, Down, Left, Right);
#endif
	}

	static void SetUIBoxPaddingSizeMode(UIBox::SizeMode Mode, UIBox* Target)
	{
#if !EDITOR
		Target->SetPaddingSizeMode(Mode);
#endif
	}

	static Vector3 GetUIBoxSize(UIBox* Target)
	{
#if !EDITOR
		return Vector3(Target->GetUsedSize(), 0);
#endif
	}

	static Vector3 GetUIBoxPosition(UIBox* Target)
	{
#if !EDITOR
		return Vector3(Target->GetPosition(), 0);
#endif
	}

	static void AddUIBoxChild(UIBox* Child, UIBox* Target)
	{
#if !EDITOR
		Target->AddChild(Child);
#endif
	}


	static UIBackground* CreateUIBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale)
	{
#if EDITOR
		return nullptr;
#endif
		return (UIBackground*)(new UIBackground(Horizontal, Position, Color, MinScale))->SetTryFill(true);
	}

	static void SetUIBackgroundColor(UIBackground* Target, Vector3 Color)
	{
#if !EDITOR
		Target->SetColor(Color);
#endif
	}

	static Vector3 GetUIBackgroundColor(UIBackground* Target)
	{
#if !EDITOR
		return Target->GetColor();
#endif
		return 0;
	}

	static void SetUIBackgroundTexture(UIBackground* Target, const char* Texture)
	{
#if !EDITOR
		Target->SetUseTexture(true, Texture);
#endif
		return;
	}

	static TextRenderer* CreateTextRenderer(const char* Font)
	{
#if EDITOR
		return nullptr;
#endif
		return new TextRenderer(Font);
	}

	static UIText* CreateUIText(float Scale, Vector3 Color, const char* Text, TextRenderer* Renderer)
	{
#if EDITOR
		return nullptr;
#endif
		return new UIText(Scale, Color, Text, Renderer);
	}

	static void SetUITextText(const char* Text, UIText* Target)
	{
#if !EDITOR
		Target->SetText(Text);
#endif
	}

	static void SetUITextColor(Vector3 Color, UIText* Target)
	{
#if !EDITOR
		Target->SetColor(Color);
#endif
	}

	static void SetCursorVisible(bool NewVisible)
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
	using namespace Error;

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
	REGISTER_FUNCTION(CollisionComponentOverlap);
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
	REGISTER_FUNCTION(PrintStackTrace);

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
	REGISTER_FUNCTION(GetUIBoxPosition);
	REGISTER_FUNCTION(GetUIBoxSize);

	REGISTER_FUNCTION(CreateUIBackground);
	REGISTER_FUNCTION(SetUIBackgroundColor);
	REGISTER_FUNCTION(GetUIBackgroundColor);
	REGISTER_FUNCTION(SetUIBackgroundTexture);

	REGISTER_FUNCTION(CreateUIText);
	REGISTER_FUNCTION(CreateTextRenderer);
	REGISTER_FUNCTION(SetUITextColor);
	REGISTER_FUNCTION(SetUITextText);
#endif

}

#endif