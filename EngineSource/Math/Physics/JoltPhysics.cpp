#include "JoltPhysics.h"
#include "Physics.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

JPH_SUPPRESS_WARNINGS

using namespace JPH;
using namespace JPH::literals;

#include <Engine/EngineError.h>
#include <Engine/Utility/StringUtility.h>
#include <cstdarg>
#include <Engine/Log.h>
#include <Engine/Stats.h>
#include <Math/Math.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <Engine/Input.h>

inline static Vec3 ToJPHVec3(const Vector3& Vec)
{
	return Vec3(Vec.X, Vec.Y, Vec.Z);
}

inline static Quat ToJPHQuat(Vector3 Rot)
{
	glm::quat rotation = glm::quat((glm::vec3)Rot.DegreesToRadians());
	return Quat(rotation.x, rotation.y, rotation.z, rotation.w);
}

struct PhysicsBodyInfo
{
	BodyID ID;
	Physics::PhysicsBody* Body = nullptr;
};
namespace JoltPhysics
{
	std::unordered_map<BodyID, PhysicsBodyInfo> Bodies;
}

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	Log::PrintMultiLine(buffer);
}

#ifndef JPH_ENABLE_ASSERTS

static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
	Error::AssertFailure("Physics system error: " + std::string(inMessage) + ", " + std::string(inExpression), 
		StrUtil::Format("%s, Line %i", inFile, inLine));

	return true;
};

#endif

static bool LayerMask(Physics::Layer Bit, Physics::Layer Mask)
{
	return (bool)(Bit & Mask);
}

class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	bool CheckForStaticDynamic(Physics::Layer inObject1, Physics::Layer inObject2) const
	{
		if (LayerMask(inObject1, Physics::Layer::Static) && LayerMask(inObject2, Physics::Layer::Dynamic))
		{
			return true;
		}

		return LayerMask(inObject1, inObject2);
	}
	virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		if (CheckForStaticDynamic((Physics::Layer)inObject1, (Physics::Layer)inObject2)) return true;
		if (CheckForStaticDynamic((Physics::Layer)inObject2, (Physics::Layer)inObject1)) return true;
		return inObject1 & inObject2;
	}
};

namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer STATIC(0);
	static constexpr BroadPhaseLayer DYNAMIC(1);
	static constexpr BroadPhaseLayer CUSTOM(2);
	static constexpr uint NUM_LAYERS = 3;
};

class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
	}

	virtual uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		Physics::Layer Layer = (Physics::Layer)inLayer;
		if ((bool)(Layer & Physics::Layer::Dynamic))
		{
			return BroadPhaseLayers::DYNAMIC;
		}
		if ((bool)(Layer & Physics::Layer::Static))
		{
			return BroadPhaseLayers::STATIC;
		}
		return BroadPhaseLayers::CUSTOM;
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		return StrUtil::Format("Layer%i", (int)inLayer.GetValue()).c_str();
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		Physics::Layer Layer = (Physics::Layer)inLayer1;
		if ((bool)(Layer & Physics::Layer::Dynamic))
		{
			return inLayer2 == BroadPhaseLayers::STATIC;
		}
		if ((bool)(Layer & Physics::Layer::Static))
		{
			return inLayer2 == BroadPhaseLayers::STATIC || inLayer2 == BroadPhaseLayers::DYNAMIC;
		}
		return inLayer2 == BroadPhaseLayers::CUSTOM;
	}
};

namespace JoltPhysics
{
	PhysicsSystem* System = nullptr;
	BodyInterface* JoltBodyInterface = nullptr;
	TempAllocatorImpl* TempAllocator = nullptr;
	JobSystemThreadPool* JobSystem = nullptr;
	BPLayerInterfaceImpl broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;
}

void JoltPhysics::Init()
{
	RegisterDefaultAllocator();

	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;);

	// Create a factory
	Factory::sInstance = new Factory();
	RegisterTypes();

	TempAllocator = new TempAllocatorImpl(1024 * 1024);
	JobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.

	const uint cMaxBodies = 65536;

	const uint cNumBodyMutexes = 0;
	const uint cMaxBodyPairs = 10240;
	const uint cMaxContactConstraints = 1024;

	System = new PhysicsSystem();
	System->Init(cMaxBodies,
		cNumBodyMutexes,
		cMaxBodyPairs,
		cMaxContactConstraints,
		broad_phase_layer_interface,
		object_vs_broadphase_layer_filter,
		object_vs_object_layer_filter);

	System->SetGravity(System->GetGravity() * 4);

	JoltBodyInterface = &System->GetBodyInterface();
}

void JoltPhysics::RegisterBody(Physics::PhysicsBody* Body)
{
	using namespace Physics;

	BodyID BodyID;

	switch (Body->Type)
	{
	case PhysicsBody::BodyType::Sphere:
	{
		SphereBody* SpherePtr = static_cast<SphereBody*>(Body);
		BodyCreationSettings SphereSettigns = BodyCreationSettings(new SphereShape(SpherePtr->GetTransform().Scale.X),
			ToJPHVec3(Body->GetTransform().Location),
			ToJPHQuat(SpherePtr->GetTransform().Rotation),
			EMotionType::Dynamic,
			(ObjectLayer)Body->CollisionLayers);
		BodyID = JoltBodyInterface->CreateAndAddBody(SphereSettigns, EActivation::Activate);
		break;
	}
	case PhysicsBody::BodyType::Mesh:
	{
		MeshBody* MeshPtr = static_cast<MeshBody*>(Body);

		VertexList Vertices;
		IndexedTriangleList Indices;

		auto MergedVertices = MeshPtr->MeshData.GetMergedVertices();
		auto MergedIndices = MeshPtr->MeshData.GetMergedIndices();

		Vertices.reserve(MergedVertices.size());
		Indices.reserve(MergedIndices.size() / 3);
		Transform OffsetTransform = MeshPtr->GetTransform();
		OffsetTransform.Location = 0;
		OffsetTransform.Rotation = Vector3(OffsetTransform.Rotation.Z, -OffsetTransform.Rotation.Y, OffsetTransform.Rotation.X).DegreesToRadians();
		OffsetTransform.Scale = OffsetTransform.Scale * Vector3(0.025f);
		glm::mat4 ModelMatrix = OffsetTransform.ToMatrix();
		for (auto& i : MergedVertices)
		{
			i.Position = ModelMatrix * glm::vec4(i.Position, 1);

			Vertices.push_back(Float3(i.Position.x, i.Position.y, i.Position.z));
		}

		for (size_t i = 0; i < MergedIndices.size(); i += 3)
		{
			IndexedTriangle Tri = IndexedTriangle((uint32)MergedIndices[i], (uint32)MergedIndices[i + 1], (uint32)MergedIndices[i + 2]);
			Indices.push_back(Tri);
		}


		MeshShapeSettings Settings = MeshShapeSettings(Vertices, Indices);
		ShapeSettings::ShapeResult MeshResult;
		MeshShape* Shape = new MeshShape(Settings, MeshResult);
		if (!MeshResult.IsValid())
		{
			Log::PrintMultiLine("Error creating collision shape: " + std::string(MeshResult.GetError()), Log::LogColor::Red);
			return;
		}

		BodyCreationSettings MeshSettings = BodyCreationSettings(Shape,
			ToJPHVec3(MeshPtr->GetTransform().Location),
			Quat::sIdentity(),
			EMotionType::Static,
			(ObjectLayer)MeshPtr->CollisionLayers);

		BodyID = JoltBodyInterface->CreateAndAddBody(MeshSettings, EActivation::Activate);
		break;
	}
	default:
		ENGINE_UNREACHABLE();
		break;
	}

	PhysicsBodyInfo info;
	info.Body = Body;
	info.ID = BodyID;
	Bodies.insert({ BodyID, info });

	Body->PhysicsSystemBody = &Bodies[BodyID];
}

void JoltPhysics::RemoveBody(Physics::PhysicsBody* Body)
{
	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->RemoveBody(Info->ID);
	JoltBodyInterface->DestroyBody(Info->ID);
	Bodies.erase(Info->ID);
}

Vector3 JoltPhysics::GetBodyPosition(Physics::PhysicsBody* Body)
{
	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	Vec3 vec = JoltBodyInterface->GetPosition(Info->ID);
	return Vector3(vec.GetX(), vec.GetY(), vec.GetZ());
}

Vector3 JoltPhysics::GetBodyRotation(Physics::PhysicsBody* Body)
{
	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	Vec3 vec = JoltBodyInterface->GetRotation(Info->ID).GetEulerAngles();
	return Vector3(vec.GetZ(), -vec.GetY(), vec.GetX()).RadiansToDegrees();
}

void JoltPhysics::Update()
{
	if (Performance::DeltaTime > 0 && (Input::IsKeyDown(Input::Key::f) || !IsInEditor))
	{
		System->Update(Performance::DeltaTime, 1, TempAllocator, JobSystem);
	}
}

Physics::HitResult JoltPhysics::LineCast(Vector3 Start, Vector3 End)
{
	RRayCast Cast = RRayCast(ToJPHVec3(Start), ToJPHVec3(End - Start));
	RayCastResult HitInfo;
	bool Hit = System->GetNarrowPhaseQuery().CastRay(Cast, HitInfo);

	Physics::HitResult r;
	r.Hit = Hit;
	if (Hit)
	{
		auto val = Bodies.find(HitInfo.mBodyID);
		if (val != Bodies.end())
		{
			PhysicsBodyInfo BodyInfo = (*val).second;
			r.HitComponent = BodyInfo.Body->Parent;
			Vec3 ImpactPos = Cast.GetPointOnRay(HitInfo.mFraction);
			r.ImpactPoint = Vector3(ImpactPos.GetX(), ImpactPos.GetY(), ImpactPos.GetZ());
			Vec3 Rot = JoltBodyInterface->GetRotation(HitInfo.mBodyID).GetEulerAngles();
		}
	}
	return r;
}
