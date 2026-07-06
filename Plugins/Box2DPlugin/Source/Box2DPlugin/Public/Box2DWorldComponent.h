#pragma once

#include "Components/ActorComponent.h"
#include "box2d/id.h"
#include "box2d/types.h"
#include "Box2DWorldComponent.generated.h"

// Forward declarations
class AActor;

UENUM(BlueprintType)
enum class EBox2DBodyType : uint8
{
    Static UMETA(DisplayName = "Static"),
    Kinematic UMETA(DisplayName = "Kinematic"),
    Dynamic UMETA(DisplayName = "Dynamic")
};

USTRUCT(BlueprintType)
struct FBox2DBodyHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 Index1 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 World0 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 Generation = 0;

    b2BodyId ToB2Id() const
    {
        b2BodyId Id;
        Id.index1 = Index1;
        Id.world0 = (uint16_t)World0;
        Id.generation = (uint16_t)Generation;
        return Id;
    }

    static FBox2DBodyHandle FromB2Id(b2BodyId Id)
    {
        FBox2DBodyHandle Handle;
        Handle.Index1 = Id.index1;
        Handle.World0 = Id.world0;
        Handle.Generation = Id.generation;
        return Handle;
    }

    bool IsValid() const { return Index1 != 0; }
};

USTRUCT(BlueprintType)
struct FBox2DShapeHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 Index1 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 World0 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 Generation = 0;

    b2ShapeId ToB2Id() const
    {
        b2ShapeId Id;
        Id.index1 = Index1;
        Id.world0 = (uint16_t)World0;
        Id.generation = (uint16_t)Generation;
        return Id;
    }

    static FBox2DShapeHandle FromB2Id(b2ShapeId Id)
    {
        FBox2DShapeHandle Handle;
        Handle.Index1 = Id.index1;
        Handle.World0 = Id.world0;
        Handle.Generation = Id.generation;
        return Handle;
    }

    bool IsValid() const { return Index1 != 0; }
};

USTRUCT(BlueprintType)
struct FBox2DJointHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 Index1 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 World0 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 Generation = 0;

    b2JointId ToB2Id() const
    {
        b2JointId Id;
        Id.index1 = Index1;
        Id.world0 = (uint16_t)World0;
        Id.generation = (uint16_t)Generation;
        return Id;
    }

    static FBox2DJointHandle FromB2Id(b2JointId Id)
    {
        FBox2DJointHandle Handle;
        Handle.Index1 = Id.index1;
        Handle.World0 = Id.world0;
        Handle.Generation = Id.generation;
        return Handle;
    }

    bool IsValid() const { return Index1 != 0; }
};

// Body move event data for Blueprint delegate
USTRUCT(BlueprintType)
struct FBox2DBodyMoveEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    FBox2DBodyHandle BodyHandle;

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    FVector2D Position = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    float Rotation = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    bool bFellAsleep = false;
};

// Contact event data for Blueprint delegates
USTRUCT(BlueprintType)
struct FBox2DContactEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    FBox2DShapeHandle ShapeA;

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    FBox2DShapeHandle ShapeB;
};

// Contact hit event data with approach speed
USTRUCT(BlueprintType)
struct FBox2DContactHitEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    FBox2DShapeHandle ShapeA;

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    FBox2DShapeHandle ShapeB;

    UPROPERTY(BlueprintReadOnly, Category = Box2D)
    float ApproachSpeed = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBodyMoved, const FBox2DBodyMoveEvent&, MoveEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContactBegin, const FBox2DContactEvent&, ContactEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContactEnd, const FBox2DContactEvent&, ContactEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContactHit, const FBox2DContactHitEvent&, HitEvent);

UCLASS(ClassGroup=(Box2D), meta=(BlueprintSpawnableComponent))
class BOX2DPLUGIN_API UBox2DWorldComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBox2DWorldComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // World properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    FVector2D Gravity = FVector2D(0.0f, -9.81f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 SubStepCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    float FixedTimeStep = 1.0f / 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    bool bEnableSleep = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    bool bEnableContinuous = true;

    // Auto-sync: if true, Actors set via SetBodyOwnerActor will have their
    // transforms updated automatically each physics step.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    bool bAutoSyncOwnerActors = true;

    // Scale factor for Box2D → UE position mapping (meters to UE units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    float PixelsPerMeter = 100.0f;

    // Depth (Z) value when mapping Box2D 2D position to UE 3D
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    float DepthValue = 0.0f;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Box2D|Events")
    FOnBodyMoved OnBodyMoved;

    UPROPERTY(BlueprintAssignable, Category = "Box2D|Events")
    FOnContactBegin OnContactBegin;

    UPROPERTY(BlueprintAssignable, Category = "Box2D|Events")
    FOnContactEnd OnContactEnd;

    UPROPERTY(BlueprintAssignable, Category = "Box2D|Events")
    FOnContactHit OnContactHit;

    // Blueprint API - World
    UFUNCTION(BlueprintCallable, Category = "Box2D|World")
    void SetGravity(FVector2D NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Box2D|World")
    void SetEnableSleep(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Box2D|World")
    FVector2D GetGravity();

    // Blueprint API - Body
    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    FBox2DBodyHandle CreateBody(EBox2DBodyType BodyType, FVector2D Position, float Rotation = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void DestroyBody(FBox2DBodyHandle BodyHandle);

    UFUNCTION(BlueprintPure, Category = "Box2D|Body")
    FVector2D GetBodyPosition(FBox2DBodyHandle BodyHandle);

    UFUNCTION(BlueprintPure, Category = "Box2D|Body")
    float GetBodyRotation(FBox2DBodyHandle BodyHandle);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void SetBodyTransform(FBox2DBodyHandle BodyHandle, FVector2D Position, float Rotation);

    // Drive a kinematic body toward a target transform over the next time step
    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void SetBodyTargetTransform(FBox2DBodyHandle BodyHandle, FVector2D Position, float Rotation, float TimeStep);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void SetBodyLinearVelocity(FBox2DBodyHandle BodyHandle, FVector2D Velocity);

    UFUNCTION(BlueprintPure, Category = "Box2D|Body")
    FVector2D GetBodyLinearVelocity(FBox2DBodyHandle BodyHandle);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void ApplyForce(FBox2DBodyHandle BodyHandle, FVector2D Force, FVector2D Point, bool bWake = true);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void ApplyForceToCenter(FBox2DBodyHandle BodyHandle, FVector2D Force, bool bWake = true);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void ApplyLinearImpulse(FBox2DBodyHandle BodyHandle, FVector2D Impulse, FVector2D Point, bool bWake = true);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void ApplyLinearImpulseToCenter(FBox2DBodyHandle BodyHandle, FVector2D Impulse, bool bWake = true);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void ApplyTorque(FBox2DBodyHandle BodyHandle, float Torque, bool bWake = true);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void SetBodyAngularVelocity(FBox2DBodyHandle BodyHandle, float AngularVelocity);

    UFUNCTION(BlueprintPure, Category = "Box2D|Body")
    float GetBodyAngularVelocity(FBox2DBodyHandle BodyHandle);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void WakeBody(FBox2DBodyHandle BodyHandle);

    // Body-Actor association
    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void SetBodyOwnerActor(FBox2DBodyHandle BodyHandle, AActor* Owner);

    UFUNCTION(BlueprintPure, Category = "Box2D|Body")
    AActor* GetBodyOwnerActor(FBox2DBodyHandle BodyHandle);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Body")
    void SetBodyUserData(FBox2DBodyHandle BodyHandle, int64 UserData);

    UFUNCTION(BlueprintPure, Category = "Box2D|Body")
    int64 GetBodyUserData(FBox2DBodyHandle BodyHandle);

    // Blueprint API - Shape
    UFUNCTION(BlueprintCallable, Category = "Box2D|Shape")
    FBox2DShapeHandle CreateCircleShape(FBox2DBodyHandle BodyHandle, float Radius, FVector2D Center, float Density = 1.0f, float Friction = 0.6f, float Restitution = 0.0f, float TangentSpeed = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Shape")
    FBox2DShapeHandle CreatePolygonShape(FBox2DBodyHandle BodyHandle, const TArray<FVector2D>& Vertices, FVector2D Center, float Rotation, float Density = 1.0f, float Friction = 0.6f, float Restitution = 0.0f, float TangentSpeed = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Shape")
    FBox2DShapeHandle CreateBoxShape(FBox2DBodyHandle BodyHandle, float HalfWidth, float HalfHeight, FVector2D Center, float Rotation = 0.0f, float Density = 1.0f, float Friction = 0.6f, float Restitution = 0.0f, float TangentSpeed = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Shape")
    void DestroyShape(FBox2DShapeHandle ShapeHandle);

    // Blueprint API - Joint: Distance
    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    FBox2DJointHandle CreateDistanceJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
        float Length = 0.0f, bool bEnableSpring = false, float Hertz = 0.0f, float DampingRatio = 0.0f,
        bool bEnableLimit = false, float MinLength = 0.0f, float MaxLength = 0.0f,
        bool bEnableMotor = false, float MaxMotorForce = 0.0f, float MotorSpeed = 0.0f,
        bool bCollideConnected = false);

    // Blueprint API - Joint: Revolute (hinge/pin)
    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    FBox2DJointHandle CreateRevoluteJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
        FVector2D AnchorA = FVector2D::ZeroVector, FVector2D AnchorB = FVector2D::ZeroVector,
        bool bEnableSpring = false, float Hertz = 0.0f, float DampingRatio = 0.0f,
        bool bEnableLimit = false, float LowerAngle = 0.0f, float UpperAngle = 0.0f,
        bool bEnableMotor = false, float MaxMotorTorque = 0.0f, float MotorSpeed = 0.0f,
        bool bCollideConnected = false);

    // Blueprint API - Joint: Prismatic (slider)
    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    FBox2DJointHandle CreatePrismaticJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
        FVector2D AnchorA = FVector2D::ZeroVector, FVector2D AnchorB = FVector2D::ZeroVector,
        FVector2D LocalAxisA = FVector2D(1.0f, 0.0f),
        bool bEnableSpring = false, float Hertz = 0.0f, float DampingRatio = 0.0f,
        bool bEnableLimit = false, float LowerTranslation = 0.0f, float UpperTranslation = 0.0f,
        bool bEnableMotor = false, float MaxMotorForce = 0.0f, float MotorSpeed = 0.0f,
        bool bCollideConnected = false);

    // Blueprint API - Joint: Weld
    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    FBox2DJointHandle CreateWeldJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
        FVector2D AnchorA = FVector2D::ZeroVector, FVector2D AnchorB = FVector2D::ZeroVector,
        float LinearHertz = 0.0f, float LinearDampingRatio = 0.0f,
        float AngularHertz = 0.0f, float AngularDampingRatio = 0.0f,
        bool bCollideConnected = false);

    // Blueprint API - Joint: Wheel
    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    FBox2DJointHandle CreateWheelJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
        FVector2D Anchor = FVector2D::ZeroVector,
        FVector2D LocalAxisA = FVector2D(0.0f, 1.0f),
        bool bEnableSpring = true, float Hertz = 4.0f, float DampingRatio = 0.7f,
        bool bEnableLimit = false, float LowerTranslation = 0.0f, float UpperTranslation = 0.0f,
        bool bEnableMotor = false, float MaxMotorTorque = 0.0f, float MotorSpeed = 0.0f,
        bool bCollideConnected = false);

    // Blueprint API - Joint: Motor
    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    FBox2DJointHandle CreateMotorJoint(FBox2DBodyHandle BodyA, FBox2DBodyHandle BodyB,
        FVector2D LinearVelocity = FVector2D::ZeroVector, float AngularVelocity = 0.0f,
        float MaxForce = 0.0f, float MaxTorque = 0.0f,
        float LinearHertz = 0.0f, float LinearDampingRatio = 0.0f,
        float AngularHertz = 0.0f, float AngularDampingRatio = 0.0f,
        bool bCollideConnected = false);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    void DestroyJoint(FBox2DJointHandle JointHandle, bool bWakeAttached = true);

    // Blueprint API - Joint queries
    UFUNCTION(BlueprintPure, Category = "Box2D|Joint")
    float GetRevoluteJointAngle(FBox2DJointHandle JointHandle);

    UFUNCTION(BlueprintPure, Category = "Box2D|Joint")
    float GetPrismaticJointTranslation(FBox2DJointHandle JointHandle);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    void SetRevoluteJointMotorSpeed(FBox2DJointHandle JointHandle, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    void SetPrismaticJointMotorSpeed(FBox2DJointHandle JointHandle, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    void SetWheelJointMotorSpeed(FBox2DJointHandle JointHandle, float Speed);

    UFUNCTION(BlueprintCallable, Category = "Box2D|Joint")
    void WakeJoint(FBox2DJointHandle JointHandle);

    // Blueprint API - Shape UserData
    UFUNCTION(BlueprintCallable, Category = "Box2D|Shape")
    void SetShapeUserData(FBox2DShapeHandle ShapeHandle, int64 UserData);

    UFUNCTION(BlueprintPure, Category = "Box2D|Shape")
    int64 GetShapeUserData(FBox2DShapeHandle ShapeHandle);

    // Blueprint API - Event processing
    UFUNCTION(BlueprintCallable, Category = "Box2D|Events")
    void ProcessBodyMoveEvents();

    UFUNCTION(BlueprintCallable, Category = "Box2D|Events")
    void ProcessContactEvents();

    // Access to raw world id for renderer
    b2WorldId GetWorldId() const { return WorldId; }
    bool IsWorldValid() const;

    // Destroy the current b2 world and create a fresh one
    UFUNCTION(BlueprintCallable, Category = "Box2D|World")
    void ResetWorld();

private:
    b2WorldId WorldId;
};
