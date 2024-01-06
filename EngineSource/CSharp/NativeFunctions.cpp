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
#include <Engine/Gamepad.h>
#include <Engine/Application.h>
#include <cstring>

char* CopyString(const char* s)
{
	size_t len = 1 + strlen(s);
	char* p = (char*)malloc(len);

	return p ? (char*)memcpy(p, s, len) : nullptr;
}

#if SERVER
class UIBox;
class UIBackground;
class UIButton;
class UIText;
class TextRenderer;
#endif

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
		NewCollider->Load(m.GetMergedVertices(), m.GetMergedIndices());
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

	static char* GetTypeNameOfObject(WorldObject* Object)
	{
		return CopyString(Object->GetObjectDescription().Name.c_str());
	}

	static void DestroyObject(WorldObject* Ptr)
	{
		Objects::DestroyObject(Ptr);
	}

	static void SetObjectName(WorldObject* Object, const char* NewName)
	{
		Object->Name = NewName;
	}

	static char* GetObjectName(WorldObject* Object)
	{
		// The .NET runtime (apparently) frees this.
		return CopyString(Object->Name.c_str());
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
		auto hit = Collision::LineTrace(Start, End, { ObjectsToIgnore });
		return hit;
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

#pragma region UI
	static UIBox* CreateUIBox(bool Horizontal, Vector2 Position)
	{
#if !EDITOR && !SERVER
		return new UIBox(Horizontal ? UIBox::Orientation::Horizontal : UIBox::Orientation::Vertical, Position);
#endif
		return nullptr;
	}

	static void DestroyUIBox(UIBox* Target)
	{
#if !EDITOR && !SERVER
		delete Target;
#endif
	}

	static void SetUIBoxMinSize(Vector2 NewMinSize, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetMinSize(NewMinSize);
#endif
	}

	static void SetUIBoxMaxSize(Vector2 NewMaxSize, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetMaxSize(NewMaxSize);
#endif
	}

	static void SetUIBoxPosition(Vector2 Position, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetPosition(Position);
#endif
	}

	static void SetUIBoxVerticalAlign(int NewAlign, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetVerticalAlign((UIBox::Align)NewAlign);
#endif
	}

	static void SetUIBoxHorizontalAlign(int NewAlign, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetHorizontalAlign((UIBox::Align)NewAlign);
#endif
	}

	static void SetUIBoxSizeMode(int Mode, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetSizeMode((UIBox::SizeMode)Mode);
#endif
	}

	static void SetUIBoxBorder(int NewBorder, float Size, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetBorder((UIBox::BorderType)NewBorder, Size);
#endif
	}

	static void SetUIBoxPadding(float Up, float Down, float Left, float Right, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetPadding(Up, Down, Left, Right);
#endif
	}

	static void SetUIBoxPaddingSizeMode(int Mode, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->SetPaddingSizeMode((UIBox::SizeMode)Mode);
#endif
	}

	static Vector3 GetUIBoxSize(UIBox* Target)
	{
#if !EDITOR && !SERVER
		return Vector3(Target->GetUsedSize(), 0);
#endif
		return 0;
	}

	static Vector3 GetUIBoxPosition(UIBox* Target)
	{
#if !EDITOR && !SERVER
		return Vector3(Target->GetPosition(), 0);
#endif
		return 0;
	}

	static void AddUIBoxChild(UIBox* Child, UIBox* Target)
	{
#if !EDITOR && !SERVER
		Target->AddChild(Child);
#endif
	}


	static UIBackground* CreateUIBackground(bool Horizontal, Vector2 Position, Vector3 Color, Vector2 MinScale)
	{
#if !EDITOR && !SERVER
		return (UIBackground*)(new UIBackground(Horizontal ? UIBox::Orientation::Horizontal : UIBox::Orientation::Vertical, Position, Color, MinScale))->SetTryFill(true);
#endif
		return nullptr;
	}

	static void SetUIBackgroundColor(UIBackground* Target, Vector3 Color)
	{
#if !EDITOR && !SERVER
		Target->SetColor(Color);
#endif
	}

	static Vector3 GetUIBackgroundColor(UIBackground* Target)
	{
#if !EDITOR && !SERVER
		return Target->GetColor();
#endif
		return 0;
	}

	static void SetUIBackgroundTexture(UIBackground* Target, const char* Texture)
	{
#if !EDITOR && !SERVER
		if (std::strlen(Texture) == 0)
		{
			Target->SetUseTexture(false, "");
		}
		Target->SetUseTexture(true, Texture);
#endif
		return;
	}

	static TextRenderer* CreateTextRenderer(const char* Font)
	{
#if !EDITOR && !SERVER
		return new TextRenderer(Font);
#endif
		return nullptr;
}

	static UIText* CreateUIText(float Scale, Vector3 Color, const char* Text, TextRenderer* Renderer)
	{
#if !EDITOR && !SERVER
		return new UIText(Scale, Color, Text, Renderer);
#endif
		return nullptr;
	}

	static void SetUITextText(const char* Text, UIText* Target)
	{
#if !EDITOR && !SERVER
		Target->SetText(Text);
#endif
	}

	static void SetUITextColor(Vector3 Color, UIText* Target)
	{
#if !EDITOR && !SERVER
		Target->SetColor(Color);
#endif
	}

	static void SetCursorVisible(bool NewVisible)
	{
#if !EDITOR && !SERVER
		Input::CursorVisible = NewVisible;
#endif
	}
#pragma endregion

	static int32_t GetNumGamepads()
	{
		return (int32_t)Input::Gamepads.size();
	}

	static Input::Gamepad GetGamepadIndex(int32_t Index)
	{
		int32_t it = 0;
		for (const auto& i : Input::Gamepads)
		{
			if (Index == it++)
			{
				Input::Gamepad g = i.second;
				g.DeviceName = i.second.DeviceName;
				return g;
			}
		}
		return Input::Gamepad();
	}

	static Type::TypeEnum GetObjectPropertyType(WorldObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			if (i.Name == Property)
			{
				return i.Type;
			}
		}
		return Type::Null;
	}

	static const char* GetObjectPropertyString(WorldObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			if (i.Name == Property)
			{
				return (*(std::string*)i.Data).c_str();
			}
		}
		return "";
	}

}

#define REGISTER_FUNCTION(func) CSharp::RegisterNativeFunction(# func, (void*)func)

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
	REGISTER_FUNCTION(SetObjectName);
	REGISTER_FUNCTION(GetObjectName);
	REGISTER_FUNCTION(GetTypeNameOfObject);

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

	REGISTER_FUNCTION(GetNumGamepads);
	REGISTER_FUNCTION(GetGamepadIndex);

	REGISTER_FUNCTION(GetObjectPropertyType);
	REGISTER_FUNCTION(GetObjectPropertyString);
}

#endif