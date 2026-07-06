#pragma once

#include "CoreMinimal.h"
#include "Box2DWorldComponent.h"
#include "Box2DCollisionTypes.generated.h"

// Shape types supported for Box2D collision
UENUM(BlueprintType)
enum class EBox2DCollisionShapeType : uint8
{
    Box UMETA(DisplayName = "Box"),
    Circle UMETA(DisplayName = "Circle"),
    Polygon UMETA(DisplayName = "Polygon")
};

// Joint types supported for Box2D constraints
UENUM(BlueprintType)
enum class EBox2DCollisionJointType : uint8
{
    Distance UMETA(DisplayName = "Distance"),
    Revolute UMETA(DisplayName = "Revolute"),
    Prismatic UMETA(DisplayName = "Prismatic"),
    Weld UMETA(DisplayName = "Weld"),
    Wheel UMETA(DisplayName = "Wheel"),
    Motor UMETA(DisplayName = "Motor")
};

// Projection axis for 3D-to-2D mapping
UENUM(BlueprintType)
enum class EBox2DProjectionAxis : uint8
{
    XY UMETA(DisplayName = "XY Plane"),
    XZ UMETA(DisplayName = "XZ Plane"),
    YZ UMETA(DisplayName = "YZ Plane")
};

// A single collision shape definition
USTRUCT(BlueprintType)
struct FBox2DCollisionShape
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    EBox2DCollisionShapeType ShapeType = EBox2DCollisionShapeType::Box;

    // Vertices for polygon (empty for Box/Circle)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    TArray<FVector2D> Vertices;

    // Half-extents for Box (X=half-width, Y=half-height), or radius in X for Circle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    FVector2D HalfExtents = FVector2D(0.5f, 0.5f);

    // Center offset from body origin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    FVector2D Center = FVector2D::ZeroVector;

    // Rotation in degrees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    float Rotation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    float Friction = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    float Restitution = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Shape)
    bool bIsSensor = false;

    bool IsShapeValid() const
    {
        if (ShapeType == EBox2DCollisionShapeType::Polygon)
        {
            return Vertices.Num() >= 3;
        }
        if (ShapeType == EBox2DCollisionShapeType::Circle)
        {
            return HalfExtents.X > 0.0f;
        }
        // Box
        return HalfExtents.X > 0.0f && HalfExtents.Y > 0.0f;
    }
};

// A body definition with its shapes
USTRUCT(BlueprintType)
struct FBox2DCollisionBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    FName BodyName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    EBox2DBodyType BodyType = EBox2DBodyType::Static;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    FVector2D Position = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    float Rotation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    float LinearDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    bool bFixedRotation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    bool bBullet = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Body)
    TArray<FBox2DCollisionShape> Shapes;
};

// A joint/constraint definition between two bodies
USTRUCT(BlueprintType)
struct FBox2DCollisionJoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    FName JointName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    EBox2DCollisionJointType JointType = EBox2DCollisionJointType::Revolute;

    // Reference bodies by name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    FName BodyAName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    FName BodyBName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    FVector2D AnchorA = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    FVector2D AnchorB = FVector2D::ZeroVector;

    // Axis for prismatic/wheel joints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    FVector2D LocalAxisA = FVector2D(1.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Joint)
    bool bCollideConnected = false;

    // Spring parameters (Distance, Revolute, Prismatic, Wheel)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Spring")
    bool bEnableSpring = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Spring")
    float Hertz = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Spring")
    float DampingRatio = 0.0f;

    // Limit parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Limit")
    bool bEnableLimit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Limit")
    float LowerLimit = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Limit")
    float UpperLimit = 0.0f;

    // Motor parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Motor")
    bool bEnableMotor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Motor")
    float MaxMotorForce = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Motor")
    float MaxMotorTorque = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Motor")
    float MotorSpeed = 0.0f;

    // Distance joint specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Distance")
    float Length = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Distance")
    float MinLength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Distance")
    float MaxLength = 0.0f;

    // Weld joint specific
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Weld")
    float LinearHertz = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Weld")
    float LinearDampingRatio = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Weld")
    float AngularHertz = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Joint|Weld")
    float AngularDampingRatio = 0.0f;
};
