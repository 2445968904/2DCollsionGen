#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/ObjectMacros.h"
#include "Box2DCollisionTypes.h"
#include "Box2DCollisionProfile.generated.h"

class UStaticMesh;
class USkeletalMesh;

// Asset storing a complete Box2D collision definition
UCLASS(BlueprintType, HideCategories=(Object))
class BOX2DPLUGIN_API UBox2DCollisionProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float PixelsPerMeter = 100.0f;

    // Optional source mesh for auto-generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Source)
    TSoftObjectPtr<UStaticMesh> SourceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Source)
    TSoftObjectPtr<USkeletalMesh> SourceSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Source)
    EBox2DProjectionAxis ProjectionAxis = EBox2DProjectionAxis::XY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collision)
    TArray<FBox2DCollisionBody> Bodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Collision)
    TArray<FBox2DCollisionJoint> Joints;

    // Find a body by name, returns INDEX_NONE if not found
    int32 FindBodyIndex(FName InBodyName) const
    {
        for (int32 i = 0; i < Bodies.Num(); i++)
        {
            if (Bodies[i].BodyName == InBodyName)
            {
                return i;
            }
        }
        return INDEX_NONE;
    }

    // Add a default body and return its index
    int32 AddDefaultBody(FName InBodyName = NAME_None)
    {
        FBox2DCollisionBody NewBody;
        NewBody.BodyName = InBodyName;
        if (NewBody.BodyName == NAME_None)
        {
            NewBody.BodyName = *FString::Printf(TEXT("Body_%d"), Bodies.Num());
        }
        NewBody.BodyType = EBox2DBodyType::Dynamic;

        // Add a default box shape
        FBox2DCollisionShape DefaultShape;
        DefaultShape.ShapeType = EBox2DCollisionShapeType::Box;
        DefaultShape.HalfExtents = FVector2D(0.5f, 0.5f);
        NewBody.Shapes.Add(DefaultShape);

        Bodies.Add(NewBody);
        return Bodies.Num() - 1;
    }

    // Remove a body by index and any joints referencing it
    void RemoveBody(int32 BodyIndex)
    {
        if (!Bodies.IsValidIndex(BodyIndex)) return;

        FName RemovedName = Bodies[BodyIndex].BodyName;
        Bodies.RemoveAt(BodyIndex);

        // Remove joints that referenced this body
        Joints.RemoveAll([RemovedName](const FBox2DCollisionJoint& Joint)
        {
            return Joint.BodyAName == RemovedName || Joint.BodyBName == RemovedName;
        });
    }
};
