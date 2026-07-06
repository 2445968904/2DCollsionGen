#pragma once

#include "ProceduralMeshComponent.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"
#include "Box2DWorldComponent.h"
#include "Box2DRendererComponent.generated.h"

UENUM(BlueprintType)
enum class EBox2DRenderMode : uint8
{
    Solid UMETA(DisplayName = "Solid Mesh"),
    Debug UMETA(DisplayName = "Debug Draw"),
    Both UMETA(DisplayName = "Both")
};

UCLASS(ClassGroup=(Box2D), meta=(BlueprintSpawnableComponent))
class BOX2DPLUGIN_API UBox2DRendererComponent : public UProceduralMeshComponent
{
    GENERATED_BODY()

public:
    UBox2DRendererComponent(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void FinishDestroy() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    EBox2DRenderMode RenderMode = EBox2DRenderMode::Both;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    float PixelsPerMeter = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    float DepthOffset = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    int32 CircleSegments = 32;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    float DebugLineThickness = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    float JointDrawScale = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Box2D)
    UMaterialInterface* CustomMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    bool bDrawShapes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    bool bDrawJoints = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    bool bDrawBounds = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    bool bDrawContactPoints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    bool bDrawContactNormals = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    bool bDrawAABBs = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    FLinearColor StaticBodyColor = FLinearColor(0.5f, 0.9f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    FLinearColor DynamicBodyColor = FLinearColor(0.9f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    FLinearColor KinematicBodyColor = FLinearColor(0.5f, 0.5f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Box2D|Debug")
    FLinearColor JointColor = FLinearColor(0.8f, 0.8f, 0.4f, 1.0f);

protected:
    void UpdateRenderer();
    void UpdateSolidMesh();
    void UpdateDebugDraw();

    void GenerateCircleVertices(const b2Vec2& Center, float Radius, float Depth,
        TArray<FVector>& Vertices, TArray<int32>& Indices, TArray<FVector>& Normals,
        TArray<FVector2D>& UVs, TArray<FColor>& Colors, int32& BaseIndex);

    void GeneratePolygonVertices(const b2Vec2* B2Vertices, int32 Count, float Depth,
        TArray<FVector>& Vertices, TArray<int32>& Indices, TArray<FVector>& Normals,
        TArray<FVector2D>& UVs, TArray<FColor>& Colors, int32& BaseIndex);

    void GenerateCapsuleVertices(const b2Vec2& P1, const b2Vec2& P2, float Radius, float Depth,
        TArray<FVector>& Vertices, TArray<int32>& Indices, TArray<FVector>& Normals,
        TArray<FVector2D>& UVs, TArray<FColor>& Colors, int32& BaseIndex);

    void GenerateSegmentVertices(const b2Vec2& P1, const b2Vec2& P2, float Depth,
        TArray<FVector>& Vertices, TArray<int32>& Indices, TArray<FVector>& Normals,
        TArray<FVector2D>& UVs, TArray<FColor>& Colors, int32& BaseIndex);

    FVector B2ToUE(const b2Vec2& Pos, float Depth = 0.0f) const;
    FVector B2PosToUE(const b2Pos& Pos, float Depth = 0.0f) const;
    FColor GetBodyColor(b2BodyId BodyId) const;

    void SetupDebugDraw(b2DebugDraw& Draw);

    void EnsureMaterial();

private:
    UPROPERTY()
    UMaterialInstanceDynamic* DefaultMaterial;

    UPROPERTY()
    UBox2DWorldComponent* CachedWorldComponent;
};
