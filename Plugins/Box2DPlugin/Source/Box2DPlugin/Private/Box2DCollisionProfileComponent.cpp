#include "Box2DPluginPCH.h"
#include "Box2DPlugin.h"
#include "Box2DCollisionProfileComponent.h"
#include "Engine.h"
#include "GameFramework/Actor.h"

UBox2DCollisionProfileComponent::UBox2DCollisionProfileComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoActivate = true;
}

void UBox2DCollisionProfileComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoCreateOnBeginPlay && CollisionProfile)
    {
        InstantiateProfile();
    }
}

void UBox2DCollisionProfileComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyInstantiatedBodies();
    Super::EndPlay(EndPlayReason);
}

UBox2DWorldComponent* UBox2DCollisionProfileComponent::FindWorldComponent()
{
    if (CachedWorldComponent && !CachedWorldComponent->IsBeingDestroyed())
    {
        return CachedWorldComponent;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    CachedWorldComponent = Cast<UBox2DWorldComponent>(Owner->GetComponentByClass(UBox2DWorldComponent::StaticClass()));
    return CachedWorldComponent;
}

void UBox2DCollisionProfileComponent::InstantiateProfile()
{
    if (!CollisionProfile) return;

    UBox2DWorldComponent* WorldComp = FindWorldComponent();
    if (!WorldComp || !WorldComp->IsWorldValid())
    {
        UE_LOG(Box2DLog, Warning, TEXT("Box2DCollisionProfileComponent: No valid Box2DWorldComponent found"));
        return;
    }

    DestroyInstantiatedBodies();

    // Create bodies and shapes
    for (const FBox2DCollisionBody& BodyDef : CollisionProfile->Bodies)
    {
        FBox2DBodyHandle BodyHandle = WorldComp->CreateBody(BodyDef.BodyType, BodyDef.Position, BodyDef.Rotation);
        if (!BodyHandle.IsValid()) continue;

        BodyHandleMap.Add(BodyDef.BodyName, BodyHandle);

        // Create shapes for this body
        for (const FBox2DCollisionShape& ShapeDef : BodyDef.Shapes)
        {
            switch (ShapeDef.ShapeType)
            {
            case EBox2DCollisionShapeType::Box:
                WorldComp->CreateBoxShape(BodyHandle, ShapeDef.HalfExtents.X, ShapeDef.HalfExtents.Y,
                    ShapeDef.Center, FMath::DegreesToRadians(ShapeDef.Rotation),
                    ShapeDef.Density, ShapeDef.Friction, ShapeDef.Restitution);
                break;

            case EBox2DCollisionShapeType::Circle:
                WorldComp->CreateCircleShape(BodyHandle, ShapeDef.HalfExtents.X, ShapeDef.Center,
                    ShapeDef.Density, ShapeDef.Friction, ShapeDef.Restitution);
                break;

            case EBox2DCollisionShapeType::Polygon:
                if (ShapeDef.Vertices.Num() >= 3)
                {
                    WorldComp->CreatePolygonShape(BodyHandle, ShapeDef.Vertices, ShapeDef.Center,
                        FMath::DegreesToRadians(ShapeDef.Rotation),
                        ShapeDef.Density, ShapeDef.Friction, ShapeDef.Restitution);
                }
                break;
            }
        }
    }

    // Create joints (resolve body references by name)
    for (const FBox2DCollisionJoint& JointDef : CollisionProfile->Joints)
    {
        FBox2DBodyHandle* HandleA = BodyHandleMap.Find(JointDef.BodyAName);
        FBox2DBodyHandle* HandleB = BodyHandleMap.Find(JointDef.BodyBName);
        if (!HandleA || !HandleB || !HandleA->IsValid() || !HandleB->IsValid()) continue;

        FBox2DJointHandle JointHandle;

        switch (JointDef.JointType)
        {
        case EBox2DCollisionJointType::Distance:
            JointHandle = WorldComp->CreateDistanceJoint(*HandleA, *HandleB,
                JointDef.Length, JointDef.bEnableSpring, JointDef.Hertz, JointDef.DampingRatio,
                JointDef.bEnableLimit, JointDef.MinLength, JointDef.MaxLength,
                JointDef.bEnableMotor, JointDef.MaxMotorForce, JointDef.MotorSpeed,
                JointDef.bCollideConnected);
            break;

        case EBox2DCollisionJointType::Revolute:
            JointHandle = WorldComp->CreateRevoluteJoint(*HandleA, *HandleB,
                JointDef.AnchorA, JointDef.AnchorB,
                JointDef.bEnableSpring, JointDef.Hertz, JointDef.DampingRatio,
                JointDef.bEnableLimit, JointDef.LowerLimit, JointDef.UpperLimit,
                JointDef.bEnableMotor, JointDef.MaxMotorTorque, JointDef.MotorSpeed,
                JointDef.bCollideConnected);
            break;

        case EBox2DCollisionJointType::Prismatic:
            JointHandle = WorldComp->CreatePrismaticJoint(*HandleA, *HandleB,
                JointDef.AnchorA, JointDef.AnchorB, JointDef.LocalAxisA,
                JointDef.bEnableSpring, JointDef.Hertz, JointDef.DampingRatio,
                JointDef.bEnableLimit, JointDef.LowerLimit, JointDef.UpperLimit,
                JointDef.bEnableMotor, JointDef.MaxMotorForce, JointDef.MotorSpeed,
                JointDef.bCollideConnected);
            break;

        case EBox2DCollisionJointType::Weld:
            JointHandle = WorldComp->CreateWeldJoint(*HandleA, *HandleB,
                JointDef.AnchorA, JointDef.AnchorB,
                JointDef.LinearHertz, JointDef.LinearDampingRatio,
                JointDef.AngularHertz, JointDef.AngularDampingRatio,
                JointDef.bCollideConnected);
            break;

        case EBox2DCollisionJointType::Wheel:
            JointHandle = WorldComp->CreateWheelJoint(*HandleA, *HandleB,
                JointDef.AnchorA, JointDef.LocalAxisA,
                JointDef.bEnableSpring, JointDef.Hertz, JointDef.DampingRatio,
                JointDef.bEnableLimit, JointDef.LowerLimit, JointDef.UpperLimit,
                JointDef.bEnableMotor, JointDef.MaxMotorTorque, JointDef.MotorSpeed,
                JointDef.bCollideConnected);
            break;

        case EBox2DCollisionJointType::Motor:
            JointHandle = WorldComp->CreateMotorJoint(*HandleA, *HandleB,
                FVector2D(JointDef.MotorSpeed, 0.0f), JointDef.UpperLimit,
                JointDef.MaxMotorForce, JointDef.MaxMotorTorque,
                JointDef.LinearHertz, JointDef.LinearDampingRatio,
                JointDef.AngularHertz, JointDef.AngularDampingRatio,
                JointDef.bCollideConnected);
            break;
        }

        if (JointHandle.IsValid())
        {
            CreatedJointHandles.Add(JointHandle);
        }
    }

    bIsInstantiated = true;
}

void UBox2DCollisionProfileComponent::DestroyInstantiatedBodies()
{
    if (!bIsInstantiated) return;

    UBox2DWorldComponent* WorldComp = FindWorldComponent();

    // Destroy joints first
    if (WorldComp && WorldComp->IsWorldValid())
    {
        for (const FBox2DJointHandle& JointHandle : CreatedJointHandles)
        {
            WorldComp->DestroyJoint(JointHandle, false);
        }

        // Destroy bodies
        for (const auto& Pair : BodyHandleMap)
        {
            WorldComp->DestroyBody(Pair.Value);
        }
    }

    CreatedJointHandles.Empty();
    BodyHandleMap.Empty();
    bIsInstantiated = false;
}

FBox2DBodyHandle UBox2DCollisionProfileComponent::GetBodyHandle(FName BodyName) const
{
    const FBox2DBodyHandle* Handle = BodyHandleMap.Find(BodyName);
    return Handle ? *Handle : FBox2DBodyHandle();
}
