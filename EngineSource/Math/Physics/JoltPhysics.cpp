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
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/ShapeCast.h>

#include <Engine/Subsystem/PhysicsSubsystem.h>

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
#include <Objects/Components/Component.h>
#include <glm/mat4x4.hpp>

inline static Vec3 ToJPHVec3(const Vector3& Vec)
{
	return Vec3(Vec.X, Vec.Y, Vec.Z);
}

inline static Vec4 ToJPHVec4(const glm::vec4& Vec)
{
	return Vec4(Vec.x, Vec.y, Vec.z, Vec.w);
}

inline static Quat ToJPHQuat(Vector3 Rot)
{
	Rot = Rot.DegreesToRadians();
	return Quat::sEulerAngles(Vec3(Rot.Z, -Rot.Y, Rot.X));
}

struct PhysicsBodyInfo
{
	BodyID ID;
	Shape* BodyShape = nullptr;
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

	PhysicsSubsystem::PhysicsSystem->Print(buffer, Subsystem::ErrorLevel::Error);
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
	static bool CheckForStaticDynamic(Physics::Layer inObject1, Physics::Layer inObject2)
	{
		if (LayerMask(inObject1, Physics::Layer::Static) && LayerMask(inObject2, Physics::Layer::Dynamic))
		{
			return true;
		}

		return LayerMask(inObject1, inObject2);
	}
	
	static bool ShouldLayersCollide(ObjectLayer inObject1, ObjectLayer inObject2)
	{
		if (CheckForStaticDynamic((Physics::Layer)inObject1, (Physics::Layer)inObject2)) return true;
		if (CheckForStaticDynamic((Physics::Layer)inObject2, (Physics::Layer)inObject1)) return true;
		return inObject1 & inObject2;
	}

	virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		return ShouldLayersCollide(inObject1, inObject2);
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
		if ((bool)(Layer & Physics::Layer::Static))
		{
			return BroadPhaseLayers::STATIC;
		}
		if ((bool)(Layer & Physics::Layer::Dynamic))
		{
			return BroadPhaseLayers::DYNAMIC;
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

static EMotionType ConvertMovability(Physics::MotionType Movability)
{
	return (EMotionType)Movability;
}

BodyCreationSettings CreateJoltShapeFromBody(Physics::PhysicsBody* Body)
{
	using namespace Physics;

	switch (Body->Type)
	{
	case PhysicsBody::BodyType::Sphere:
	{
		SphereBody* SpherePtr = static_cast<SphereBody*>(Body);
		return BodyCreationSettings(new SphereShape(SpherePtr->BodyTransform.Scale.X),
			ToJPHVec3(Body->BodyTransform.Position),
			ToJPHQuat(SpherePtr->BodyTransform.Rotation.RadiansToDegrees()),
			ConvertMovability(Body->ColliderMovability),
			(ObjectLayer)Body->CollisionLayers);
	}
	case PhysicsBody::BodyType::Box:
	{
		BoxBody* BoxPtr = static_cast<BoxBody*>(Body);
		return BodyCreationSettings(new BoxShape(ToJPHVec3(BoxPtr->BodyTransform.Scale)),
			ToJPHVec3(Body->BodyTransform.Position),
			ToJPHQuat(BoxPtr->BodyTransform.Rotation.RadiansToDegrees()),
			ConvertMovability(Body->ColliderMovability),
			(ObjectLayer)Body->CollisionLayers);
	}
	case PhysicsBody::BodyType::Capsule:
	{
		CapsuleBody* CapsulePtr = static_cast<CapsuleBody*>(Body);
		CapsuleShapeSettings Settings = CapsuleShapeSettings(Body->BodyTransform.Scale.X, Body->BodyTransform.Scale.Y);
		Shape::ShapeResult r;
		return BodyCreationSettings(new CapsuleShape(Settings, r),
			ToJPHVec3(Body->BodyTransform.Position),
			ToJPHQuat(CapsulePtr->BodyTransform.Rotation.RadiansToDegrees()),
			ConvertMovability(Body->ColliderMovability),
			(ObjectLayer)Body->CollisionLayers);
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
		Transform OffsetTransform = MeshPtr->BodyTransform;
		OffsetTransform.Position = 0;
		OffsetTransform.Rotation = 0;
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
			PhysicsSubsystem::PhysicsSystem->Print("Error creating collision shape: " + std::string(MeshResult.GetError()), Subsystem::ErrorLevel::Error);
			return BodyCreationSettings();
		}

		return BodyCreationSettings(Shape,
			ToJPHVec3(MeshPtr->BodyTransform.Position),
			ToJPHQuat(MeshPtr->BodyTransform.Rotation),
			EMotionType::Static,
			(ObjectLayer)MeshPtr->CollisionLayers);
	}
	}
	ENGINE_UNREACHABLE();
	return BodyCreationSettings();
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

	if (!Body->ShapeInfo)
	{
		CreateShape(Body);
	}
	auto JoltShape = static_cast<BodyCreationSettings*>(Body->ShapeInfo);

	BodyID BodyID = JoltBodyInterface->CreateAndAddBody(*JoltShape, EActivation::Activate);

	PhysicsBodyInfo info;
	info.Body = Body;
	info.ID = BodyID;
	Bodies.insert({ BodyID, info });

	Body->PhysicsSystemBody = &Bodies[BodyID];
}

void JoltPhysics::RemoveBody(Physics::PhysicsBody* Body)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->RemoveBody(Info->ID);
	JoltBodyInterface->DestroyBody(Info->ID);
	Bodies.erase(Info->ID);
}

void JoltPhysics::CreateShape(Physics::PhysicsBody* Body)
{
	Body->ShapeInfo = new BodyCreationSettings(CreateJoltShapeFromBody(Body));
}

Vector3 JoltPhysics::GetBodyPosition(Physics::PhysicsBody* Body)
{
	if (!Body->PhysicsSystemBody)
	{
		return 0;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	Vec3 vec = JoltBodyInterface->GetPosition(Info->ID);
	return Vector3(vec.GetX(), vec.GetY(), vec.GetZ());
}

Vector3 JoltPhysics::GetBodyRotation(Physics::PhysicsBody* Body)
{
	if (!Body->PhysicsSystemBody)
	{
		return 0;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	Vec3 vec = JoltBodyInterface->GetRotation(Info->ID).GetEulerAngles();
	return Vector3(vec.GetZ(), -vec.GetY(), vec.GetX()).RadiansToDegrees();
}

Vector3 JoltPhysics::GetBodyVelocity(Physics::PhysicsBody* Body)
{
	if (!Body->PhysicsSystemBody)
	{
		return 0;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	Vec3 vec = JoltBodyInterface->GetLinearVelocity(Info->ID);
	return Vector3(vec.GetX(), vec.GetY(), vec.GetZ());
}

Vector3 JoltPhysics::GetBodyAngularVelocity(Physics::PhysicsBody* Body)
{
	if (!Body->PhysicsSystemBody)
	{
		return 0;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	Vec3 vec = JoltBodyInterface->GetAngularVelocity(Info->ID);
	return Vector3(vec.GetZ(), -vec.GetY(), vec.GetX()).RadiansToDegrees();
}

void JoltPhysics::SetBodyPosition(Physics::PhysicsBody* Body, Vector3 NewPosition)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetPosition(Info->ID, ToJPHVec3(NewPosition), EActivation::Activate);
}

void JoltPhysics::SetBodyRotation(Physics::PhysicsBody* Body, Vector3 NewRotation)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetRotation(Info->ID, ToJPHQuat(NewRotation), EActivation::Activate);
}

void JoltPhysics::MultiplyBodyScale(Physics::PhysicsBody* Body, Vector3 Scale)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	auto ShapeInfo = JoltBodyInterface->GetShape(Info->ID).GetPtr()->ScaleShape(ToJPHVec3(Scale));
	JoltBodyInterface->SetShape(Info->ID, ShapeInfo.Get(), true, EActivation::Activate);
}

void JoltPhysics::AddBodyForce(Physics::PhysicsBody* Body, Vector3 Direction, Vector3 Point)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->AddForce(Info->ID, ToJPHVec3(Direction), ToJPHVec3(Point));
}

void JoltPhysics::SetBodyVelocity(Physics::PhysicsBody* Body, Vector3 NewVelocity)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetLinearVelocity(Info->ID, ToJPHVec3(NewVelocity));
}

void JoltPhysics::SetBodyAngularVelocity(Physics::PhysicsBody* Body, Vector3 NewVelocity)
{
	if (!Body->PhysicsSystemBody)
	{
		return;
	}

	PhysicsBodyInfo* Info = static_cast<PhysicsBodyInfo*>(Body->PhysicsSystemBody);
	JoltBodyInterface->SetAngularVelocity(Info->ID, ToJPHVec3(NewVelocity.DegreesToRadians()));
}

void JoltPhysics::Update()
{
#if !EDITOR
	System->Update(Performance::DeltaTime, 1, TempAllocator, JobSystem);
#endif
}

class CollisionShapeCollectorImpl : public CollideShapeCollector
{
public:

	CollisionShapeCollectorImpl()
	{

	}

	std::vector<Physics::HitResult> Hits;

	virtual void AddHit(const ResultType& inResult) override
	{
		using namespace JoltPhysics;

		Physics::HitResult r;
		r.Hit = true;
		r.Depth = inResult.mPenetrationDepth;
		r.Normal = 0 - Vector3(inResult.mPenetrationAxis.GetX(), inResult.mPenetrationAxis.GetY(), inResult.mPenetrationAxis.GetZ()).Normalize();
		r.ImpactPoint = Vector3(inResult.mContactPointOn2.GetX(), inResult.mContactPointOn2.GetY(), inResult.mContactPointOn2.GetZ());

		auto val = Bodies.find(inResult.mBodyID2);
		PhysicsBodyInfo BodyInfo = (*val).second;
		r.HitComponent = BodyInfo.Body->Parent;
		Hits.push_back(r);
	};
};

class CastShapeCollectorImpl : public CastShapeCollector
{
public:
	CastShapeCollectorImpl()
	{

	}

	std::vector<Physics::HitResult> Hits;

	virtual void AddHit(const ResultType& inResult) override
	{
		using namespace JoltPhysics;

		Physics::HitResult r;
		r.Hit = true;
		r.Depth = inResult.mPenetrationDepth;
		r.Normal = 0 - Vector3(inResult.mPenetrationAxis.GetX(), inResult.mPenetrationAxis.GetY(), inResult.mPenetrationAxis.GetZ()).Normalize();
		r.ImpactPoint = Vector3(inResult.mContactPointOn2.GetX(), inResult.mContactPointOn2.GetY(), inResult.mContactPointOn2.GetZ());
		r.Distance = inResult.mFraction;

		auto val = Bodies.find(inResult.mBodyID2);
		PhysicsBodyInfo BodyInfo = (*val).second;
		r.HitComponent = BodyInfo.Body->Parent;
		Hits.push_back(r);
	}
};

class ObjectLayerFilterImpl : public ObjectLayerFilter
{
public:
	Physics::Layer ObjLayer = Physics::Layer::Static;
	virtual bool ShouldCollide(ObjectLayer inLayer) const override
	{
		return ObjectLayerPairFilterImpl::ShouldLayersCollide((ObjectLayer)ObjLayer, inLayer);
	}
};

class BodyFilterImpl : public BodyFilter
{
public:
	std::set<WorldObject*>* ObjectsToIgnore = nullptr;

	virtual bool ShouldCollide(const BodyID& inObject) const override
	{
		auto body = JoltPhysics::Bodies.find(inObject);
		if (body != JoltPhysics::Bodies.end()
			&& body->second.Body->Parent
			&& body->second.Body->Parent->GetParent())
		{
			return ObjectsToIgnore->find(body->second.Body->Parent->GetParent()) == ObjectsToIgnore->end();
		}
		return true;
	}
};

std::vector<Physics::HitResult> JoltPhysics::CollisionTest(Physics::PhysicsBody* Body, Physics::Layer Layers, std::set<WorldObject*> ObjectsToIgnore)
{
	using namespace Physics;

	if (!Body->ShapeInfo)
	{
		CreateShape(Body);
	}
	auto JoltShape = static_cast<BodyCreationSettings*>(Body->ShapeInfo);

	Transform t = Body->BodyTransform;
	t.Scale = 1;
	glm::mat4 mat = t.ToMatrix();

	Mat44 ResultMat = Mat44(ToJPHVec4(mat[0]), ToJPHVec4(mat[1]), ToJPHVec4(mat[2]), ToJPHVec4(mat[3]));

	CollisionShapeCollectorImpl cl;
	CollideShapeSettings Settings = CollideShapeSettings();
	Settings.mPenetrationTolerance = 0;
	Settings.mCollisionTolerance = 0;

	BroadPhaseLayerFilter BplF;
	ObjectLayerFilterImpl LayerF;
	LayerF.ObjLayer = Layers;
	BodyFilterImpl ObjF;
	ObjF.ObjectsToIgnore = &ObjectsToIgnore;

	System->GetNarrowPhaseQuery().CollideShape(JoltShape->GetShape(), ToJPHVec3(1), ResultMat, Settings, Vec3(), cl, BplF, LayerF, ObjF);
	
	return cl.Hits;
}

std::vector<Physics::HitResult> JoltPhysics::ShapeCastBody(Physics::PhysicsBody* Body, Transform StartPos, Vector3 EndPos, Physics::Layer Layers, std::set<WorldObject*> ObjectsToIgnore)
{
	using namespace Physics;

	if (!Body->ShapeInfo)
	{
		CreateShape(Body);
	}

	auto JoltShape = static_cast<BodyCreationSettings*>(Body->ShapeInfo);

	glm::mat4 mat = StartPos.ToMatrix();

	Vector3 Direction = EndPos - StartPos.Position;
	
	Mat44 ResultMat = Mat44(ToJPHVec4(mat[0]), ToJPHVec4(mat[1]), ToJPHVec4(mat[2]), ToJPHVec4(mat[3]));

	CastShapeCollectorImpl cl;
	ShapeCastSettings s;

	BroadPhaseLayerFilter BplF;
	ObjectLayerFilterImpl LayerF;
	LayerF.ObjLayer = Layers;
	BodyFilterImpl ObjF;
	ObjF.ObjectsToIgnore = &ObjectsToIgnore;

	RShapeCast c = RShapeCast(JoltShape->GetShape(), ToJPHVec3(StartPos.Scale), ResultMat, ToJPHVec3(Direction));
	System->GetNarrowPhaseQuery().CastShape(c, s, Vec3(0, 0, 0), cl, BplF, LayerF, ObjF);
	return cl.Hits;

}

Physics::HitResult JoltPhysics::LineCast(Vector3 Start, Vector3 End, Physics::Layer Layers, std::set<WorldObject*> ObjectsToIgnore)
{
	RRayCast Cast = RRayCast(ToJPHVec3(Start), ToJPHVec3(End - Start));
	RayCastResult HitInfo;

	BroadPhaseLayerFilter BplF;
	ObjectLayerFilterImpl LayerF;
	LayerF.ObjLayer = Layers;
	BodyFilterImpl ObjF;
	ObjF.ObjectsToIgnore = &ObjectsToIgnore;
	bool Hit = System->GetNarrowPhaseQuery().CastRay(Cast, HitInfo, BplF, LayerF, ObjF);

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

			Vec3 Normal = JoltBodyInterface->GetTransformedShape(BodyInfo.ID).GetWorldSpaceSurfaceNormal(HitInfo.mSubShapeID2, ImpactPos);
			r.Distance = HitInfo.mFraction;
			r.Normal = Vector3(Normal.GetX(), Normal.GetY(), Normal.GetZ());
		}
	}
	return r;
}
