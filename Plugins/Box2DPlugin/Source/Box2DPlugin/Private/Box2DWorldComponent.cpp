#include "Box2DPluginPCH.h"
#include "Box2DPlugin.h"
#include "Box2DWorldComponent.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "box2d/box2d.h"

// ToB2/FromB2/ToB2Pos moved to Box2DPluginPCH.h

#define LOCTEXT_NAMESPACE "Box2D"

UBox2DWorldComponent::UBox2DWorldComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoActivate = true;
    WorldId = b2_nullWorldId;
}

void UBox2DWorldComponent::BeginPlay()
{
    Super::BeginPlay();

    b2WorldDef WorldDef = b2DefaultWorldDef();
    WorldDef.gravity = ToB2(Gravity);
    WorldDef.enableSleep = bEnableSleep;
    WorldDef.enableContinuous = bEnableContinuous;

    WorldId = b2CreateWorld(&WorldDef);

    if (B2_IS_NULL(WorldId))
    {
        UE_LOG(Box2DLog, Error, TEXT("Failed to create Box2D world"));
    }
    else
    {
        UE_LOG(Box2DLog, Log, TEXT("Box2D world created successfully"));
    }
}

void UBox2DWorldComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (B2_IS_NON_NULL(WorldId))
    {
        b2DestroyWorld(WorldId);
        WorldId = b2_nullWorldId;
        UE_LOG(Box2DLog, Log, TEXT("Box2D world destroyed"));
    }

    Super::EndPlay(EndPlayReason);
}

void UBox2DWorldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (B2_IS_NON_NULL(WorldId))
    {
        b2World_Step(WorldId, FixedTimeStep, SubStepCount);
        ProcessBodyMoveEvents();
        ProcessContactEvents();
    }
}

bool UBox2DWorldComponent::IsWorldValid() const
{
    return B2_IS_NON_NULL(WorldId) && b2World_IsValid(WorldId);
}

void UBox2DWorldComponent::ResetWorld()
{
    if (B2_IS_NON_NULL(WorldId))
    {
        b2DestroyWorld(WorldId);
        WorldId = b2_nullWorldId;
    }

    b2WorldDef WorldDef = b2DefaultWorldDef();
    WorldDef.gravity = ToB2(Gravity);
    WorldDef.enableSleep = bEnableSleep;
    WorldDef.enableContinuous = bEnableContinuous;

    WorldId = b2CreateWorld(&WorldDef);

    if (B2_IS_NULL(WorldId))
    {
        UE_LOG(Box2DLog, Error, TEXT("Failed to recreate Box2D world"));
    }
    else
    {
        UE_LOG(Box2DLog, Log, TEXT("Box2D world reset successfully"));
    }
}

// World API
void UBox2DWorldComponent::SetGravity(FVector2D NewGravity)
{
    Gravity = NewGravity;
    if (B2_IS_NON_NULL(WorldId))
    {
        b2World_SetGravity(WorldId, ToB2(NewGravity));
    }
}

void UBox2DWorldComponent::SetEnableSleep(bool bEnable)
{
    bEnableSleep = bEnable;
    if (B2_IS_NON_NULL(WorldId))
    {
        b2World_EnableSleeping(WorldId, bEnable);
    }
}

FVector2D UBox2DWorldComponent::GetGravity()
{
    if (B2_IS_NON_NULL(WorldId))
    {
        return FromB2(b2World_GetGravity(WorldId));
    }
    return Gravity;
}

// Body API
FBox2DBodyHandle UBox2DWorldComponent::CreateBody(EBox2DBodyType BodyType, FVector2D Position, float Rotation)
{
    if (B2_IS_NULL(WorldId)) return FBox2DBodyHandle();

    b2BodyDef BodyDef = b2DefaultBodyDef();
    switch (BodyType)
    {
    case EBox2DBodyType::Static:    BodyDef.type = b2_staticBody; break;
    case EBox2DBodyType::Kinematic: BodyDef.type = b2_kinematicBody; break;
    case EBox2DBodyType::Dynamic:   BodyDef.type = b2_dynamicBody; break;
    }

    BodyDef.position = ToB2Pos(Position);
    BodyDef.rotation = b2MakeRot(Rotation);

    b2BodyId BodyId = b2CreateBody(WorldId, &BodyDef);
    return FBox2DBodyHandle::FromB2Id(BodyId);
}

void UBox2DWorldComponent::DestroyBody(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (b2Body_IsValid(BodyId))
    {
        b2DestroyBody(BodyId);
    }
}

FVector2D UBox2DWorldComponent::GetBodyPosition(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return FVector2D::ZeroVector;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return FVector2D::ZeroVector;

    b2Pos Pos = b2Body_GetPosition(BodyId);
    return FVector2D((float)Pos.x, (float)Pos.y);
}

float UBox2DWorldComponent::GetBodyRotation(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return 0.0f;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return 0.0f;

    b2Rot Rot = b2Body_GetRotation(BodyId);
    return b2Rot_GetAngle(Rot);
}

void UBox2DWorldComponent::SetBodyTransform(FBox2DBodyHandle BodyHandle, FVector2D Position, float Rotation)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_SetTransform(BodyId, ToB2Pos(Position), b2MakeRot(Rotation));
}

void UBox2DWorldComponent::SetBodyTargetTransform(FBox2DBodyHandle BodyHandle, FVector2D Position, float Rotation, float TimeStep)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2WorldTransform Target = { ToB2Pos(Position), b2MakeRot(Rotation) };
    b2Body_SetTargetTransform(BodyId, Target, TimeStep, true);
}

void UBox2DWorldComponent::SetBodyLinearVelocity(FBox2DBodyHandle BodyHandle, FVector2D Velocity)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_SetLinearVelocity(BodyId, ToB2(Velocity));
}

FVector2D UBox2DWorldComponent::GetBodyLinearVelocity(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return FVector2D::ZeroVector;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return FVector2D::ZeroVector;

    return FromB2(b2Body_GetLinearVelocity(BodyId));
}

void UBox2DWorldComponent::ApplyForce(FBox2DBodyHandle BodyHandle, FVector2D Force, FVector2D Point, bool bWake)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_ApplyForce(BodyId, ToB2(Force), ToB2Pos(Point), bWake);
}

void UBox2DWorldComponent::ApplyForceToCenter(FBox2DBodyHandle BodyHandle, FVector2D Force, bool bWake)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_ApplyForceToCenter(BodyId, ToB2(Force), bWake);
}

void UBox2DWorldComponent::ApplyLinearImpulse(FBox2DBodyHandle BodyHandle, FVector2D Impulse, FVector2D Point, bool bWake)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_ApplyLinearImpulse(BodyId, ToB2(Impulse), ToB2Pos(Point), bWake);
}

// Shape API
FBox2DShapeHandle UBox2DWorldComponent::CreateCircleShape(FBox2DBodyHandle BodyHandle, float Radius, FVector2D Center, float Density, float Friction, float Restitution, float TangentSpeed)
{
    if (!BodyHandle.IsValid()) return FBox2DShapeHandle();
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return FBox2DShapeHandle();

    b2ShapeDef ShapeDef = b2DefaultShapeDef();
    ShapeDef.density = Density;
    ShapeDef.material.friction = Friction;
    ShapeDef.material.restitution = Restitution;
    ShapeDef.material.tangentSpeed = TangentSpeed;
    ShapeDef.enableContactEvents = true;

    b2Circle Circle = { ToB2(Center), Radius };
    b2ShapeId ShapeId = b2CreateCircleShape(BodyId, &ShapeDef, &Circle);

    return FBox2DShapeHandle::FromB2Id(ShapeId);
}

FBox2DShapeHandle UBox2DWorldComponent::CreatePolygonShape(FBox2DBodyHandle BodyHandle, const TArray<FVector2D>& Vertices, FVector2D Center, float Rotation, float Density, float Friction, float Restitution, float TangentSpeed)
{
    if (!BodyHandle.IsValid()) return FBox2DShapeHandle();
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return FBox2DShapeHandle();

    b2ShapeDef ShapeDef = b2DefaultShapeDef();
    ShapeDef.density = Density;
    ShapeDef.material.friction = Friction;
    ShapeDef.material.restitution = Restitution;
    ShapeDef.material.tangentSpeed = TangentSpeed;
    ShapeDef.enableContactEvents = true;

    int32 Count = FMath::Min(Vertices.Num(), B2_MAX_POLYGON_VERTICES);
    b2Vec2 B2Vertices[8];
    for (int32 i = 0; i < Count; i++)
    {
        B2Vertices[i] = ToB2(Vertices[i]);
    }

    b2Hull Hull = b2ComputeHull(B2Vertices, Count);
    if (Hull.count == 0)
    {
        UE_LOG(Box2DLog, Warning, TEXT("Failed to compute hull for polygon shape"));
        return FBox2DShapeHandle();
    }

    b2Polygon Polygon = b2MakePolygon(&Hull, 0.0f);
    b2ShapeId ShapeId = b2CreatePolygonShape(BodyId, &ShapeDef, &Polygon);

    return FBox2DShapeHandle::FromB2Id(ShapeId);
}

FBox2DShapeHandle UBox2DWorldComponent::CreateBoxShape(FBox2DBodyHandle BodyHandle, float HalfWidth, float HalfHeight, FVector2D Center, float Rotation, float Density, float Friction, float Restitution, float TangentSpeed)
{
    if (!BodyHandle.IsValid()) return FBox2DShapeHandle();
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return FBox2DShapeHandle();

    b2ShapeDef ShapeDef = b2DefaultShapeDef();
    ShapeDef.density = Density;
    ShapeDef.material.friction = Friction;
    ShapeDef.material.restitution = Restitution;
    ShapeDef.material.tangentSpeed = TangentSpeed;
    ShapeDef.enableContactEvents = true;

    b2Polygon Box = b2MakeOffsetBox(HalfWidth, HalfHeight, ToB2(Center), b2MakeRot(Rotation));
    b2ShapeId ShapeId = b2CreatePolygonShape(BodyId, &ShapeDef, &Box);

    return FBox2DShapeHandle::FromB2Id(ShapeId);
}

void UBox2DWorldComponent::DestroyShape(FBox2DShapeHandle ShapeHandle)
{
    if (!ShapeHandle.IsValid()) return;
    b2ShapeId ShapeId = ShapeHandle.ToB2Id();
    if (b2Shape_IsValid(ShapeId))
    {
        b2DestroyShape(ShapeId, true);
    }
}

// ==================== Joint API ====================

FBox2DJointHandle UBox2DWorldComponent::CreateDistanceJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
    float Length, bool bEnableSpring, float Hertz, float DampingRatio,
    bool bEnableLimit, float MinLength, float MaxLength,
    bool bEnableMotor, float MaxMotorForce, float MotorSpeed,
    bool bCollideConnected)
{
    if (!BodyA.IsValid() || !BodyB.IsValid()) return {};
    if (B2_IS_NULL(WorldId)) return {};

    b2BodyId BA = BodyA.ToB2Id();
    b2BodyId BB = BodyB.ToB2Id();
    if (!b2Body_IsValid(BA) || !b2Body_IsValid(BB)) return {};

    b2DistanceJointDef Def = b2DefaultDistanceJointDef();
    Def.base.bodyIdA = BA;
    Def.base.bodyIdB = BB;
    Def.base.collideConnected = bCollideConnected;
    Def.length = Length;
    Def.enableSpring = bEnableSpring;
    Def.hertz = Hertz;
    Def.dampingRatio = DampingRatio;
    Def.enableLimit = bEnableLimit;
    Def.minLength = MinLength;
    Def.maxLength = MaxLength;
    Def.enableMotor = bEnableMotor;
    Def.maxMotorForce = MaxMotorForce;
    Def.motorSpeed = MotorSpeed;

    b2JointId JointId = b2CreateDistanceJoint(WorldId, &Def);
    return FBox2DJointHandle::FromB2Id(JointId);
}

FBox2DJointHandle UBox2DWorldComponent::CreateRevoluteJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
    FVector2D AnchorA, FVector2D AnchorB,
    bool bEnableSpring, float Hertz, float DampingRatio,
    bool bEnableLimit, float LowerAngle, float UpperAngle,
    bool bEnableMotor, float MaxMotorTorque, float MotorSpeed,
    bool bCollideConnected)
{
    if (!BodyA.IsValid() || !BodyB.IsValid()) return {};
    if (B2_IS_NULL(WorldId)) return {};

    b2BodyId BA = BodyA.ToB2Id();
    b2BodyId BB = BodyB.ToB2Id();
    if (!b2Body_IsValid(BA) || !b2Body_IsValid(BB)) return {};

    b2RevoluteJointDef Def = b2DefaultRevoluteJointDef();
    Def.base.bodyIdA = BA;
    Def.base.bodyIdB = BB;
    Def.base.localFrameA.p = b2Body_GetLocalPoint(BA, ToB2Pos(AnchorA));
    Def.base.localFrameA.q = b2Rot_identity;
    Def.base.localFrameB.p = b2Body_GetLocalPoint(BB, ToB2Pos(AnchorB));
    Def.base.localFrameB.q = b2Rot_identity;
    Def.base.collideConnected = bCollideConnected;
    Def.enableSpring = bEnableSpring;
    Def.hertz = Hertz;
    Def.dampingRatio = DampingRatio;
    Def.enableLimit = bEnableLimit;
    Def.lowerAngle = FMath::DegreesToRadians(LowerAngle);
    Def.upperAngle = FMath::DegreesToRadians(UpperAngle);
    Def.enableMotor = bEnableMotor;
    Def.maxMotorTorque = MaxMotorTorque;
    Def.motorSpeed = FMath::DegreesToRadians(MotorSpeed);

    b2JointId JointId = b2CreateRevoluteJoint(WorldId, &Def);
    return FBox2DJointHandle::FromB2Id(JointId);
}

FBox2DJointHandle UBox2DWorldComponent::CreatePrismaticJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
    FVector2D AnchorA, FVector2D AnchorB, FVector2D LocalAxisA,
    bool bEnableSpring, float Hertz, float DampingRatio,
    bool bEnableLimit, float LowerTranslation, float UpperTranslation,
    bool bEnableMotor, float MaxMotorForce, float MotorSpeed,
    bool bCollideConnected)
{
    if (!BodyA.IsValid() || !BodyB.IsValid()) return {};
    if (B2_IS_NULL(WorldId)) return {};

    b2BodyId BA = BodyA.ToB2Id();
    b2BodyId BB = BodyB.ToB2Id();
    if (!b2Body_IsValid(BA) || !b2Body_IsValid(BB)) return {};

    b2PrismaticJointDef Def = b2DefaultPrismaticJointDef();
    Def.base.bodyIdA = BA;
    Def.base.bodyIdB = BB;
    Def.base.localFrameA.p = b2Body_GetLocalPoint(BA, ToB2Pos(AnchorA));
    Def.base.localFrameA.q = b2MakeRot(FMath::Atan2((float)ToB2(LocalAxisA).y, (float)ToB2(LocalAxisA).x));
    Def.base.localFrameB.p = b2Body_GetLocalPoint(BB, ToB2Pos(AnchorB));
    Def.base.localFrameB.q = b2Rot_identity;
    Def.base.collideConnected = bCollideConnected;
    Def.enableSpring = bEnableSpring;
    Def.hertz = Hertz;
    Def.dampingRatio = DampingRatio;
    Def.enableLimit = bEnableLimit;
    Def.lowerTranslation = LowerTranslation;
    Def.upperTranslation = UpperTranslation;
    Def.enableMotor = bEnableMotor;
    Def.maxMotorForce = MaxMotorForce;
    Def.motorSpeed = MotorSpeed;

    b2JointId JointId = b2CreatePrismaticJoint(WorldId, &Def);
    return FBox2DJointHandle::FromB2Id(JointId);
}

FBox2DJointHandle UBox2DWorldComponent::CreateWeldJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
    FVector2D AnchorA, FVector2D AnchorB,
    float LinearHertz, float LinearDampingRatio,
    float AngularHertz, float AngularDampingRatio,
    bool bCollideConnected)
{
    if (!BodyA.IsValid() || !BodyB.IsValid()) return {};
    if (B2_IS_NULL(WorldId)) return {};

    b2BodyId BA = BodyA.ToB2Id();
    b2BodyId BB = BodyB.ToB2Id();
    if (!b2Body_IsValid(BA) || !b2Body_IsValid(BB)) return {};

    b2WeldJointDef Def = b2DefaultWeldJointDef();
    Def.base.bodyIdA = BA;
    Def.base.bodyIdB = BB;
    Def.base.localFrameA.p = b2Body_GetLocalPoint(BA, ToB2Pos(AnchorA));
    Def.base.localFrameA.q = b2Rot_identity;
    Def.base.localFrameB.p = b2Body_GetLocalPoint(BB, ToB2Pos(AnchorB));
    Def.base.localFrameB.q = b2Rot_identity;
    Def.base.collideConnected = bCollideConnected;
    Def.linearHertz = LinearHertz;
    Def.linearDampingRatio = LinearDampingRatio;
    Def.angularHertz = AngularHertz;
    Def.angularDampingRatio = AngularDampingRatio;

    b2JointId JointId = b2CreateWeldJoint(WorldId, &Def);
    return FBox2DJointHandle::FromB2Id(JointId);
}

FBox2DJointHandle UBox2DWorldComponent::CreateWheelJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
    FVector2D Anchor, FVector2D LocalAxisA,
    bool bEnableSpring, float Hertz, float DampingRatio,
    bool bEnableLimit, float LowerTranslation, float UpperTranslation,
    bool bEnableMotor, float MaxMotorTorque, float MotorSpeed,
    bool bCollideConnected)
{
    if (!BodyA.IsValid() || !BodyB.IsValid()) return {};
    if (B2_IS_NULL(WorldId)) return {};

    b2BodyId BA = BodyA.ToB2Id();
    b2BodyId BB = BodyB.ToB2Id();
    if (!b2Body_IsValid(BA) || !b2Body_IsValid(BB)) return {};

    b2WheelJointDef Def = b2DefaultWheelJointDef();
    Def.base.bodyIdA = BA;
    Def.base.bodyIdB = BB;
    b2Pos Pivot = ToB2Pos(Anchor);
    Def.base.localFrameA.p = b2Body_GetLocalPoint(BA, Pivot);
    Def.base.localFrameA.q = b2MakeRot(FMath::Atan2((float)ToB2(LocalAxisA).y, (float)ToB2(LocalAxisA).x));
    Def.base.localFrameB.p = b2Body_GetLocalPoint(BB, Pivot);
    Def.base.localFrameB.q = b2Rot_identity;
    Def.base.collideConnected = bCollideConnected;
    Def.enableSpring = bEnableSpring;
    Def.hertz = Hertz;
    Def.dampingRatio = DampingRatio;
    Def.enableLimit = bEnableLimit;
    Def.lowerTranslation = LowerTranslation;
    Def.upperTranslation = UpperTranslation;
    Def.enableMotor = bEnableMotor;
    Def.maxMotorTorque = MaxMotorTorque;
    Def.motorSpeed = FMath::DegreesToRadians(MotorSpeed);

    b2JointId JointId = b2CreateWheelJoint(WorldId, &Def);
    return FBox2DJointHandle::FromB2Id(JointId);
}

FBox2DJointHandle UBox2DWorldComponent::CreateMotorJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
    FVector2D LinearVelocity, float AngularVelocity,
    float MaxForce, float MaxTorque,
    float LinearHertz, float LinearDampingRatio,
    float AngularHertz, float AngularDampingRatio,
    bool bCollideConnected)
{
    if (!BodyA.IsValid() || !BodyB.IsValid()) return {};
    if (B2_IS_NULL(WorldId)) return {};

    b2BodyId BA = BodyA.ToB2Id();
    b2BodyId BB = BodyB.ToB2Id();
    if (!b2Body_IsValid(BA) || !b2Body_IsValid(BB)) return {};

    b2MotorJointDef Def = b2DefaultMotorJointDef();
    Def.base.bodyIdA = BA;
    Def.base.bodyIdB = BB;
    Def.base.collideConnected = bCollideConnected;
    Def.linearVelocity = ToB2(LinearVelocity);
    Def.angularVelocity = FMath::DegreesToRadians(AngularVelocity);
    Def.maxVelocityForce = MaxForce;
    Def.maxVelocityTorque = MaxTorque;
    Def.linearHertz = LinearHertz;
    Def.linearDampingRatio = LinearDampingRatio;
    Def.angularHertz = AngularHertz;
    Def.angularDampingRatio = AngularDampingRatio;

    b2JointId JointId = b2CreateMotorJoint(WorldId, &Def);
    return FBox2DJointHandle::FromB2Id(JointId);
}

void UBox2DWorldComponent::DestroyJoint(FBox2DJointHandle JointHandle, bool bWakeAttached)
{
    if (!JointHandle.IsValid()) return;
    b2JointId JointId = JointHandle.ToB2Id();
    if (b2Joint_IsValid(JointId))
    {
        b2DestroyJoint(JointId, bWakeAttached);
    }
}

float UBox2DWorldComponent::GetRevoluteJointAngle(FBox2DJointHandle JointHandle)
{
    if (!JointHandle.IsValid()) return 0.0f;
    b2JointId JointId = JointHandle.ToB2Id();
    if (!b2Joint_IsValid(JointId)) return 0.0f;
    return FMath::RadiansToDegrees(b2RevoluteJoint_GetAngle(JointId));
}

float UBox2DWorldComponent::GetPrismaticJointTranslation(FBox2DJointHandle JointHandle)
{
    if (!JointHandle.IsValid()) return 0.0f;
    b2JointId JointId = JointHandle.ToB2Id();
    if (!b2Joint_IsValid(JointId)) return 0.0f;
    return b2PrismaticJoint_GetTranslation(JointId);
}

void UBox2DWorldComponent::SetRevoluteJointMotorSpeed(FBox2DJointHandle JointHandle, float Speed)
{
    if (!JointHandle.IsValid()) return;
    b2JointId JointId = JointHandle.ToB2Id();
    if (!b2Joint_IsValid(JointId)) return;
    b2RevoluteJoint_SetMotorSpeed(JointId, FMath::DegreesToRadians(Speed));
}

void UBox2DWorldComponent::SetPrismaticJointMotorSpeed(FBox2DJointHandle JointHandle, float Speed)
{
    if (!JointHandle.IsValid()) return;
    b2JointId JointId = JointHandle.ToB2Id();
    if (!b2Joint_IsValid(JointId)) return;
    b2PrismaticJoint_SetMotorSpeed(JointId, Speed);
}

void UBox2DWorldComponent::SetWheelJointMotorSpeed(FBox2DJointHandle JointHandle, float Speed)
{
    if (!JointHandle.IsValid()) return;
    b2JointId JointId = JointHandle.ToB2Id();
    if (!b2Joint_IsValid(JointId)) return;
    b2WheelJoint_SetMotorSpeed(JointId, Speed);
}

void UBox2DWorldComponent::WakeJoint(FBox2DJointHandle JointHandle)
{
    if (!JointHandle.IsValid()) return;
    b2JointId JointId = JointHandle.ToB2Id();
    if (!b2Joint_IsValid(JointId)) return;
    b2Joint_WakeBodies(JointId);
}

// ==================== Body API (extended) ====================

void UBox2DWorldComponent::ApplyLinearImpulseToCenter(FBox2DBodyHandle BodyHandle, FVector2D Impulse, bool bWake)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_ApplyLinearImpulseToCenter(BodyId, ToB2(Impulse), bWake);
}

void UBox2DWorldComponent::ApplyTorque(FBox2DBodyHandle BodyHandle, float Torque, bool bWake)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_ApplyTorque(BodyId, Torque, bWake);
}

void UBox2DWorldComponent::SetBodyAngularVelocity(FBox2DBodyHandle BodyHandle, float AngularVelocity)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_SetAngularVelocity(BodyId, AngularVelocity);
}

float UBox2DWorldComponent::GetBodyAngularVelocity(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return 0.0f;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return 0.0f;

    return b2Body_GetAngularVelocity(BodyId);
}

void UBox2DWorldComponent::WakeBody(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_SetAwake(BodyId, true);
}

// ==================== Body-Actor Association ====================

void UBox2DWorldComponent::SetBodyOwnerActor(FBox2DBodyHandle BodyHandle, AActor* Owner)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_SetUserData(BodyId, Owner);
}

AActor* UBox2DWorldComponent::GetBodyOwnerActor(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return nullptr;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return nullptr;

    return static_cast<AActor*>(b2Body_GetUserData(BodyId));
}

void UBox2DWorldComponent::SetBodyUserData(FBox2DBodyHandle BodyHandle, int64 UserData)
{
    if (!BodyHandle.IsValid()) return;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return;

    b2Body_SetUserData(BodyId, reinterpret_cast<void*>(UserData));
}

int64 UBox2DWorldComponent::GetBodyUserData(FBox2DBodyHandle BodyHandle)
{
    if (!BodyHandle.IsValid()) return 0;
    b2BodyId BodyId = BodyHandle.ToB2Id();
    if (!b2Body_IsValid(BodyId)) return 0;

    return reinterpret_cast<int64>(b2Body_GetUserData(BodyId));
}

// ==================== Shape UserData ====================

void UBox2DWorldComponent::SetShapeUserData(FBox2DShapeHandle ShapeHandle, int64 UserData)
{
    if (!ShapeHandle.IsValid()) return;
    b2ShapeId ShapeId = ShapeHandle.ToB2Id();
    if (!b2Shape_IsValid(ShapeId)) return;

    b2Shape_SetUserData(ShapeId, reinterpret_cast<void*>(UserData));
}

int64 UBox2DWorldComponent::GetShapeUserData(FBox2DShapeHandle ShapeHandle)
{
    if (!ShapeHandle.IsValid()) return 0;
    b2ShapeId ShapeId = ShapeHandle.ToB2Id();
    if (!b2Shape_IsValid(ShapeId)) return 0;

    return reinterpret_cast<int64>(b2Shape_GetUserData(ShapeId));
}

// ==================== Event Processing ====================

void UBox2DWorldComponent::ProcessBodyMoveEvents()
{
    if (B2_IS_NULL(WorldId)) return;

    b2BodyEvents Events = b2World_GetBodyEvents(WorldId);

    for (int i = 0; i < Events.moveCount; i++)
    {
        const b2BodyMoveEvent& B2Event = Events.moveEvents[i];
        float PosX = (float)B2Event.transform.p.x;
        float PosY = (float)B2Event.transform.p.y;
        float Angle = b2Rot_GetAngle(B2Event.transform.q);

        FBox2DBodyMoveEvent MoveEvent;
        MoveEvent.BodyHandle = FBox2DBodyHandle::FromB2Id(B2Event.bodyId);
        MoveEvent.Position = FVector2D(PosX, PosY);
        MoveEvent.Rotation = Angle;
        MoveEvent.bFellAsleep = B2Event.fellAsleep;

        // Broadcast delegate
        OnBodyMoved.Broadcast(MoveEvent);

        // Auto-sync owner Actor position
        if (bAutoSyncOwnerActors)
        {
            AActor* Owner = static_cast<AActor*>(B2Event.userData);
            if (Owner && Owner->IsValidLowLevel())
            {
                FVector NewLocation(PosX * PixelsPerMeter, DepthValue, PosY * PixelsPerMeter);
                Owner->SetActorLocation(NewLocation);
                Owner->SetActorRotation(FRotator(0.0f, FMath::RadiansToDegrees(Angle), 0.0f));
            }
        }
    }
}

void UBox2DWorldComponent::ProcessContactEvents()
{
    if (B2_IS_NULL(WorldId)) return;

    b2ContactEvents Events = b2World_GetContactEvents(WorldId);

    // Begin contact
    for (int i = 0; i < Events.beginCount; i++)
    {
        const b2ContactBeginTouchEvent& B2Event = Events.beginEvents[i];
        FBox2DContactEvent ContactEvent;
        ContactEvent.ShapeA = FBox2DShapeHandle::FromB2Id(B2Event.shapeIdA);
        ContactEvent.ShapeB = FBox2DShapeHandle::FromB2Id(B2Event.shapeIdB);
        OnContactBegin.Broadcast(ContactEvent);
    }

    // End contact
    for (int i = 0; i < Events.endCount; i++)
    {
        const b2ContactEndTouchEvent& B2Event = Events.endEvents[i];
        FBox2DContactEvent ContactEvent;
        ContactEvent.ShapeA = FBox2DShapeHandle::FromB2Id(B2Event.shapeIdA);
        ContactEvent.ShapeB = FBox2DShapeHandle::FromB2Id(B2Event.shapeIdB);
        OnContactEnd.Broadcast(ContactEvent);
    }

    // Hit contact
    for (int i = 0; i < Events.hitCount; i++)
    {
        const b2ContactHitEvent& B2Event = Events.hitEvents[i];
        FBox2DContactHitEvent HitEvent;
        HitEvent.ShapeA = FBox2DShapeHandle::FromB2Id(B2Event.shapeIdA);
        HitEvent.ShapeB = FBox2DShapeHandle::FromB2Id(B2Event.shapeIdB);
        HitEvent.ApproachSpeed = B2Event.approachSpeed;
        OnContactHit.Broadcast(HitEvent);
    }
}

#undef LOCTEXT_NAMESPACE
