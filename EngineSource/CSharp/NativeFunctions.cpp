#ifdef ENGINE_CSHARP
#include "NativeFunctions.h"
#include <Engine/Subsystem/CSharpInterop.h>
#include <Engine/EngineError.h>
#include <Engine/File/Assets.h>
#include <Rendering/Graphics.h>
#include <Engine/Log.h>
#include <Engine/Subsystem/Scene.h>
#include <Objects/Components/MeshComponent.h>
#include <Objects/Components/CollisionComponent.h>
#include <Objects/Components/CameraComponent.h>
#include <Objects/Components/ParticleComponent.h>
#include <Objects/Components/MoveComponent.h>
#include <Objects/Components/PointLightComponent.h>
#include <Objects/CSharpObject.h>
#include <Rendering/Camera/CameraShake.h>
#include <Engine/Input.h>
#include <Engine/Subsystem/Sound.h>
#include <Math/Collision/Collision.h>
#include <Engine/Subsystem/Console.h>
#include <UI/UIButton.h>
#include <UI/UICanvas.h>
#include <UI/UIBackground.h>
#include <UI/UIText.h>
#include <Engine/Gamepad.h>
#include <Engine/Application.h>
#include <cstring>
#include <Engine/Utility/StringUtility.h>
#include "CSharpUICanvas.h"
#include <Objects/Components/PhysicsComponent.h>
#include <Math/Physics/Physics.h>

static char* CopyString(const char* s)
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
	static MeshComponent* NewMeshComponent(const char* ModelFile, SceneObject* Parent)
	{
		MeshComponent* NewModel = new MeshComponent();
		Parent->Attach(NewModel);
		NewModel->Load(ModelFile);

		return NewModel;
	}

	static CollisionComponent* NewCollisionComponent(const char* ModelFile, SceneObject* Parent)
	{
		CollisionComponent* NewCollider = new CollisionComponent();
		Parent->Attach(NewCollider);
		ModelGenerator::ModelData m;
		m.LoadModelFromFile(ModelFile);
		NewCollider->Load(m);
		return NewCollider;
	}

	static CameraComponent* NewCameraComponent(float FOV, SceneObject* Parent)
	{
		CameraComponent* NewCamera = new CameraComponent();
		Parent->Attach(NewCamera);
		NewCamera->SetFOV(FOV);
		return NewCamera;
	}

	static ParticleComponent* NewParticleComponent(const char* ParticleFile, SceneObject* Parent)
	{
		ParticleComponent* Particle = new ParticleComponent();
		Parent->Attach(Particle);
		Particle->LoadParticle(ParticleFile);
		return Particle;
	}

	static MoveComponent* NewMoveComponent(SceneObject* Parent)
	{
		MoveComponent* Movement = new MoveComponent();
		Parent->Attach(Movement);
		return Movement;
	}

	static PointLightComponent* NewPointLightComponent(SceneObject* Parent)
	{
		PointLightComponent* Light = new PointLightComponent();
		Parent->Attach(Light);
		return Light;
	}

	static PhysicsComponent* NewPhysicsComponent(SceneObject* Parent, Transform t, Physics::PhysicsBody::BodyType BodyType, Physics::MotionType Movability, Physics::Layer Layers)
	{
		PhysicsComponent* c = new PhysicsComponent();
		Parent->Attach(c);
		switch (BodyType)
		{
		case Physics::PhysicsBody::BodyType::Box:
			c->CreateBox(t, Movability, Layers);
			break;
		case Physics::PhysicsBody::BodyType::Sphere:
			c->CreateSphere(t, Movability, Layers);
			break;
		case Physics::PhysicsBody::BodyType::Capsule:
			c->CreateCapsule(t, Movability, Layers);
			break;
		default:
			CSharpInterop::CSharpSystem->CSharpLog("Invalid physics component type: " + std::to_string((int)BodyType), CSharpInterop::CS_Log_Runtime, CSharpInterop::CS_Log_Warn);
			break;
		}
		return c;
	}

	static Transform PhysicsComponentGetTransform(PhysicsComponent* PhysComponent)
	{
		return PhysComponent->GetBodyWorldTransform();
	}

	static void PhysicsComponentSetPosition(PhysicsComponent* PhysComponent, Vector3 Pos)
	{
		PhysComponent->SetPosition(Pos);
	}

	static void PhysicsComponentSetRotation(PhysicsComponent* PhysComponent, Vector3 Rot)
	{
		PhysComponent->SetRotation(Rot);
	}

	static void PhysicsComponentSetScale(PhysicsComponent* PhysComponent, Vector3 Scl)
	{
		PhysComponent->SetScale(Scl);
	}

	static void PhysicsComponentSetVelocity(PhysicsComponent* PhysComponent, Vector3 Vel)
	{
		PhysComponent->SetVelocity(Vel);
	}

	static Vector3 PhysicsComponentGetVelocity(PhysicsComponent* PhysComponent)
	{
		return PhysComponent->GetVelocity();
	}

	static Vector3 PhysicsComponentGetAngularVelocity(PhysicsComponent* PhysComponent)
	{
		return PhysComponent->GetAngularVelocity();
	}

	static void PhysicsComponentSetAngularVelocity(PhysicsComponent* PhysComponent, Vector3 Vel)
	{
		PhysComponent->SetAngularVelocity(Vel);
	}

	static Collision::HitResponse PhysicsComponentCollisionCheck(
		PhysicsComponent* PhysComponent,
		Transform Where,
		Physics::Layer Layers,
		SceneObject** IgnoredObjects,
		int32_t IgnoredLength)
	{
		std::set<SceneObject*> Ignored;
		for (int32_t i = 0; i < IgnoredLength; i++)
		{
			Ignored.insert(IgnoredObjects[i]);
		}

		auto hit = PhysComponent->CollisionCheck(Where, Layers, Ignored);
		Collision::HitResponse Response;
		Response.Hit = hit.Hit;
		Response.Depth = hit.Depth;
		Response.HitComponent = hit.HitComponent;
		if (hit.HitComponent)
		{
			Response.HitObject = hit.HitComponent->GetParent();
		}
		Response.Distance = hit.Distance;
		Response.ImpactPoint = hit.ImpactPoint;
		Response.Normal = hit.Normal;
		return Response;
	}

	static Collision::HitResponse PhysicsComponentShapeCast(
		PhysicsComponent* PhysComponent,
		Transform Start,
		Vector3 End,
		Physics::Layer Layers,
		SceneObject** IgnoredObjects,
		int32_t IgnoredLength)
	{
		std::set<SceneObject*> Ignored = { PhysComponent->GetParent() };
		for (int32_t i = 0; i < IgnoredLength; i++)
		{
			Ignored.insert(IgnoredObjects[i]);
		}

		auto hit = PhysComponent->ShapeCast(Start, End, Layers, Ignored);
		Collision::HitResponse Response;
		Response.Hit = hit.Hit;
		Response.Depth = hit.Depth;
		Response.HitComponent = hit.HitComponent;
		if (hit.HitComponent)
		{
			Response.HitObject = hit.HitComponent->GetParent();
		}
		Response.Distance = hit.Distance;
		Response.ImpactPoint = hit.ImpactPoint;
		Response.Normal = hit.Normal;
		return Response;
	}

	static void PhysicsComponentSetActive(PhysicsComponent* PhysComponent, bool NewActive)
	{
		PhysComponent->SetActive(NewActive);
	}

	static void MovementComponentJump(MoveComponent* Target)
	{
		Target->Jump();
	}

	static void MoveComponentSetMoveSpeed(MoveComponent* Target, float Value)
	{
		Target->MaxSpeed = Value;
	}

	static void MoveComponentSetAcceleration(MoveComponent* Target, float Value)
	{
		Target->Acceleration = Value;
	}

	static void MoveComponentSetDeceleration(MoveComponent* Target, float Value)
	{
		Target->Deceleration = Value;
	}

	static void MoveComponentSetGravity(MoveComponent* Target, float Value)
	{
		Target->Gravity = Value;
	}

	static void MoveComponentSetJumpHeight(MoveComponent* Target, float Value)
	{
		Target->JumpHeight = Value;
	}

	static void MoveComponentSetAirAccel(MoveComponent* Target, float Value)
	{
		Target->AirAccelMultiplier = Value;
	}

	static Vector3 MoveComponentGetVelocity(MoveComponent* Target)
	{
		return Target->GetVelocity();
	}

	static bool MoveComponentIsOnGround(MoveComponent* Target)
	{
		return Target->GetIsOnGround();
	}

	static void MovementComponentAddMovementInput(Vector3 Direction, MoveComponent* Target)
	{
		Target->AddMovementInput(Direction);
	}

	static void UseCamera(CameraComponent* Cam)
	{
		Cam->Use();
	}

	static void SetCameraFOV(CameraComponent* Cam, float NewFOV)
	{
		Cam->SetFOV(NewFOV);
	}

	static float GetCameraFOV(CameraComponent* Cam)
	{
		return Cam->GetFOV();
	}

	static void SetPointLightIntensity(PointLightComponent* Target, float Intensity)
	{
		Target->SetIntensity(Intensity);
	}

	static void SetPointLightFalloff(PointLightComponent* Target, float Falloff)
	{
		Target->SetFalloff(Falloff);
	}

	static void SetPointLightColor(PointLightComponent* Target, Vector3 Color)
	{
		Target->SetColor(Color);
	}

	static float GetPointLightIntensity(PointLightComponent* Target, float Intensity)
	{
		return Target->GetIntensity();
	}

	static float GetPointLightFalloff(PointLightComponent* Target)
	{
		return Target->GetFalloff();
	}

	static Vector3 GetPointLightColor(PointLightComponent* Target)
	{
		return Target->GetColor();
	}

	static void DestroyComponent(Component* c, SceneObject* Parent)
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

	static void LoadScene(const char* SceneName)
	{
		Scene::LoadNewScene(SceneName);
	}

	static CSharpInterop::CSharpSceneObject NewCSObject(const char* TypeName, Transform ObjectTransform)
	{
		CSharpObject* NewObject = Objects::SpawnObject<CSharpObject>(ObjectTransform, 0);
		NewObject->LoadClass(TypeName);
		return NewObject->CS_Obj;
	}

	static SceneObject* NewNativeObject(const char* TypeName, Transform ObjectTransform)
	{
		SceneObject* NewObject = nullptr;
		for (const auto& i : Objects::ObjectTypes)
		{
			if (i.Name == TypeName)
			{
				NewObject = Objects::SpawnObjectFromID(i.ID, ObjectTransform);
			}
		}

		if (!NewObject)
		{
			CSharpInterop::CSharpSystem->CSharpLog(
				StrUtil::Format("Could not instantiate an object of the type '%s'. That type does not exist.", TypeName),
				CSharpInterop::CS_Log_Runtime,
				CSharpInterop::CS_Log_Err);
		}

		return NewObject;
	}

	static char* GetTypeNameOfObject(SceneObject* Object)
	{
		return CopyString(Object->GetObjectDescription().Name.c_str());
	}

	static void DestroyObject(SceneObject* Ptr)
	{
		Objects::DestroyObject(Ptr);
	}

	static void SetObjectName(SceneObject* Object, const char* NewName)
	{
		Object->Name = NewName;
	}

	static char* GetObjectName(SceneObject* Object)
	{
		// The .NET runtime (apparently) frees this.
		return CopyString(Object->Name.c_str());
	}

	static Sound::SoundBuffer* LoadSound(const char* File)
	{
		return new Sound::SoundBuffer(File);
	}

	static void PlaySound(Sound::SoundBuffer* s, float Pitch, float Volume, bool Looping)
	{
		Sound::PlaySound2D(s, Pitch, Volume, Looping);
	}
	
	static void UnloadSound(Sound::SoundBuffer* s)
	{
		delete s;
	}

	static Collision::HitResponse NativeRaycast(Vector3 Start, Vector3 End, SceneObject** IgnoredObjects, int32_t IgnoredLength)
	{
		std::set<SceneObject*> Ignored;
		for (int32_t i = 0; i < IgnoredLength; i++)
		{
			Ignored.insert(IgnoredObjects[i]);
		}
		return Collision::LineTrace(Start, End, Ignored);
	}

	static bool CallConsoleCommand(const char* cmd)
	{
		return Console::ExecuteConsoleCommand(cmd);
	}

	static Transform GetObjectTransform(SceneObject* TargetObject)
	{
		return TargetObject->GetTransform();
	}

	static void SetObjectTransform(Transform NewTransform, SceneObject* TargetObject)
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

	static UIButton* CreateUIButton(bool Horizontal, Vector2 Position, Vector3 Color, UICanvas* Parent, int Index)
	{
#if !EDITOR && !SERVER
		return new UIButton(Horizontal ? UIBox::Orientation::Horizontal : UIBox::Orientation::Vertical, Position, Color, Parent, Index);
#endif
		return nullptr;
	}

	static bool GetIsUIButtonHovered(UIButton* Button)
	{
#if !EDITOR && !SERVER
		return Button->GetIsHovered();
#endif
		return false;
	}

	static bool GetIsUIButtonPressed(UIButton* Button)
	{
#if !EDITOR && !SERVER
		return Button->GetIsPressed();
#endif
		return false;
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

	static UICanvas* NewUICanvas(void* OnClickedFunction, void* UpdateFunction, void* OnDestroyedFunction)
	{
		auto c = UICanvas::CreateNewCanvas<CSharpUICanvas>();
		if (c)
		{
			c->LoadCSharpFunctions(OnClickedFunction, UpdateFunction, OnDestroyedFunction);
		}
		return c;
	}

	static void DestroyUICanvas(UICanvas* Canvas)
	{
		if (Canvas)
		{
			delete Canvas;
		}
	}
#pragma endregion

#pragma region Input
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
				g.DeviceName = CopyString(i.second.DeviceName);
				return g;
			}
		}
		return Input::Gamepad();
	}

	static void SetCursorVisible(bool NewVisible)
	{
#if !EDITOR && !SERVER
		Input::CursorVisible = NewVisible;
#endif
	}

	static Vector3 GetMouseMovement()
	{
		return Vector3(Input::MouseMovement, 0);
	}

	static bool GetIsLMBDown()
	{
		return Input::IsLMBDown;
	}

	static bool GetIsRMBDown()
	{
		return Input::IsRMBDown;
	}

	static bool GetIsLMBClicked()
	{
		return Input::IsLMBClicked;
	}

	static bool GetIsRMBClicked()
	{
		return Input::IsRMBClicked;
	}

#pragma endregion

	static NativeType::NativeType GetObjectPropertyType(SceneObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			std::string Name = i.Name.substr(i.Name.find_last_of(":") + 1);
			StrUtil::ReplaceChar(Name, '\n', "");
			if (Name == Property)
			{
				return i.NativeType;
			}
		}
		return NativeType::Null;
	}

#pragma region Properties
	static const char* GetObjectPropertyString(SceneObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			if (i.NativeType != NativeType::String)
			{
				continue;
			}

			std::string Name = i.Name.substr(i.Name.find_last_of(":") + 1);
			StrUtil::ReplaceChar(Name, '\n', "");
			if (Name == Property)
			{
				return CopyString((*(std::string*)i.Data).c_str());
			}
		}
		return "";
	}

	static int GetObjectPropertyInt(SceneObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			if (i.NativeType != NativeType::Int)
			{
				continue;
			}

			std::string Name = i.Name.substr(i.Name.find_last_of(":") + 1);
			StrUtil::ReplaceChar(Name, '\n', "");
			if (Name == Property)
			{
				return (*(int*)i.Data);
			}
		}
		return 0;
	}

	static bool GetObjectPropertyBool(SceneObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			if (i.NativeType != NativeType::Bool)
			{
				continue;
			}

			std::string Name = i.Name.substr(i.Name.find_last_of(":") + 1);
			StrUtil::ReplaceChar(Name, '\n', "");
			if (Name == Property)
			{
				return (*(bool*)i.Data);
			}
		}
		return false;
	}

	static Vector3 GetObjectPropertyVector3(SceneObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			if (i.NativeType != NativeType::Vector3)
			{
				continue;
			}

			std::string Name = i.Name.substr(i.Name.find_last_of(":") + 1);
			StrUtil::ReplaceChar(Name, '\n', "");
			if (Name == Property)
			{
				return (*(Vector3*)i.Data);
			}
		}
		return Vector3(0);
	}

	static float GetObjectPropertyFloat(SceneObject* Obj, const char* Property)
	{
		for (auto& i : Obj->Properties)
		{
			if (i.NativeType != NativeType::Float)
			{
				continue;
			}

			std::string Name = i.Name.substr(i.Name.find_last_of(":") + 1);
			StrUtil::ReplaceChar(Name, '\n', "");
			if (Name == Property)
			{
				return (*(float*)i.Data);
			}
		}
		return 0;
	}
#pragma endregion
}

#define REGISTER_FUNCTION(func) CSharpInterop::CSharpSystem->RegisterNativeFunction(# func, (void*)func)

void NativeFunctions::RegisterNativeFunctions()
{
	using namespace Input;
	using namespace CameraShake;
	using namespace Application;
	using namespace Error;

	ENGINE_ASSERT(CSharpInterop::IsAssemblyLoaded(), "Assembly should always be loaded first before registering any native functions.");

	REGISTER_FUNCTION(NewMeshComponent);
	REGISTER_FUNCTION(NewCollisionComponent);
	REGISTER_FUNCTION(NewCameraComponent);
	REGISTER_FUNCTION(NewParticleComponent);
	REGISTER_FUNCTION(NewMoveComponent);
	REGISTER_FUNCTION(NewPhysicsComponent);
	REGISTER_FUNCTION(NewPointLightComponent);

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
	REGISTER_FUNCTION(SetCameraFOV);
	REGISTER_FUNCTION(GetCameraFOV);
	REGISTER_FUNCTION(MovementComponentAddMovementInput);
	REGISTER_FUNCTION(MovementComponentJump);
	REGISTER_FUNCTION(MoveComponentSetAcceleration);
	REGISTER_FUNCTION(MoveComponentSetDeceleration);
	REGISTER_FUNCTION(MoveComponentGetVelocity);
	REGISTER_FUNCTION(MoveComponentIsOnGround);
	REGISTER_FUNCTION(MoveComponentSetGravity);
	REGISTER_FUNCTION(MoveComponentSetMoveSpeed);
	REGISTER_FUNCTION(MoveComponentSetJumpHeight);
	REGISTER_FUNCTION(MoveComponentSetAirAccel);
	REGISTER_FUNCTION(PhysicsComponentGetTransform);
	REGISTER_FUNCTION(PhysicsComponentGetVelocity);
	REGISTER_FUNCTION(PhysicsComponentGetAngularVelocity);
	REGISTER_FUNCTION(PhysicsComponentSetPosition);
	REGISTER_FUNCTION(PhysicsComponentSetRotation);
	REGISTER_FUNCTION(PhysicsComponentSetScale);
	REGISTER_FUNCTION(PhysicsComponentSetVelocity);
	REGISTER_FUNCTION(PhysicsComponentSetAngularVelocity);
	REGISTER_FUNCTION(PhysicsComponentCollisionCheck);
	REGISTER_FUNCTION(PhysicsComponentShapeCast);
	REGISTER_FUNCTION(PhysicsComponentSetActive);
	REGISTER_FUNCTION(SetPointLightIntensity);
	REGISTER_FUNCTION(SetPointLightFalloff);
	REGISTER_FUNCTION(SetPointLightColor);
	REGISTER_FUNCTION(GetPointLightIntensity);
	REGISTER_FUNCTION(GetPointLightFalloff);
	REGISTER_FUNCTION(GetPointLightColor);

	REGISTER_FUNCTION(PlayDefaultCameraShake);
	REGISTER_FUNCTION(NewNativeObject);
	REGISTER_FUNCTION(NewCSObject);
	REGISTER_FUNCTION(LoadScene);
	REGISTER_FUNCTION(LoadSound);
	REGISTER_FUNCTION(PlaySound);
	REGISTER_FUNCTION(UnloadSound);
	REGISTER_FUNCTION(NativeRaycast);
	REGISTER_FUNCTION(CallConsoleCommand);
	REGISTER_FUNCTION(Vector3::GetScaledAxis);
	REGISTER_FUNCTION(PrintStackTrace);

	REGISTER_FUNCTION(IsKeyDown);
	REGISTER_FUNCTION(GetMouseMovement);
	REGISTER_FUNCTION(SetCursorVisible);
	REGISTER_FUNCTION(GetIsLMBDown);
	REGISTER_FUNCTION(GetIsLMBClicked);
	REGISTER_FUNCTION(GetIsRMBDown);
	REGISTER_FUNCTION(GetIsRMBClicked);

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

	REGISTER_FUNCTION(CreateUIButton);
	REGISTER_FUNCTION(GetIsUIButtonHovered);
	REGISTER_FUNCTION(GetIsUIButtonPressed);

	REGISTER_FUNCTION(CreateUIText);
	REGISTER_FUNCTION(CreateTextRenderer);
	REGISTER_FUNCTION(SetUITextColor);
	REGISTER_FUNCTION(SetUITextText);

	REGISTER_FUNCTION(NewUICanvas);
	REGISTER_FUNCTION(DestroyUICanvas);

	REGISTER_FUNCTION(GetNumGamepads);
	REGISTER_FUNCTION(GetGamepadIndex);

	REGISTER_FUNCTION(GetObjectPropertyType);
	REGISTER_FUNCTION(GetObjectPropertyString);
	REGISTER_FUNCTION(GetObjectPropertyInt);
	REGISTER_FUNCTION(GetObjectPropertyVector3);
	REGISTER_FUNCTION(GetObjectPropertyBool);
	REGISTER_FUNCTION(GetObjectPropertyFloat);
}

#endif