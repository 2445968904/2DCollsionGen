#include "Box2DPluginPCH.h"
#include "Box2DPlugin.h"
#include "Box2DRendererComponent.h"
#include "Engine.h"
#include "DrawDebugHelpers.h"
#include "box2d/box2d.h"

// ToB2/FromB2 moved to Box2DPluginPCH.h

static FColor HexColorToFColor(b2HexColor Hex)

#define LOCTEXT_NAMESPACE "Box2D"
{
    uint8 B = Hex & 0xFF;
    uint8 G = (Hex >> 8) & 0xFF;
    uint8 R = (Hex >> 16) & 0xFF;
    return FColor(R, G, B, 255);
}

#define LOCTEXT_NAMESPACE "Box2D"

UBox2DRendererComponent::UBox2DRendererComponent(const FObjectInitializer& ObjectInitializer)
    : UProceduralMeshComponent(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = true;
    bTickInEditor = true;
    bAutoActivate = true;
}

void UBox2DRendererComponent::BeginPlay() { Super::BeginPlay(); }
void UBox2DRendererComponent::FinishDestroy() { Super::FinishDestroy(); }

void UBox2DRendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateRenderer();
}

void UBox2DRendererComponent::UpdateRenderer()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (!CachedWorldComponent || CachedWorldComponent->IsBeingDestroyed())
    {
        CachedWorldComponent = Cast<UBox2DWorldComponent>(Owner->GetComponentByClass(UBox2DWorldComponent::StaticClass()));
    }

    if (!CachedWorldComponent || !CachedWorldComponent->IsWorldValid())
    {
        ClearAllMeshSections();
        return;
    }

    switch (RenderMode)
    {
    case EBox2DRenderMode::Solid:  UpdateSolidMesh(); break;
    case EBox2DRenderMode::Debug:  UpdateDebugDraw(); break;
    case EBox2DRenderMode::Both:   UpdateSolidMesh(); UpdateDebugDraw(); break;
    }
}

FVector UBox2DRendererComponent::B2ToUE(const b2Vec2& Pos, float Depth) const
{
    return FVector(Pos.x * PixelsPerMeter, Depth, Pos.y * PixelsPerMeter);
}

FVector UBox2DRendererComponent::B2PosToUE(const b2Pos& Pos, float Depth) const
{
    return FVector((float)Pos.x * PixelsPerMeter, Depth, (float)Pos.y * PixelsPerMeter);
}

FColor UBox2DRendererComponent::GetBodyColor(b2BodyId BodyId) const
{
    switch (b2Body_GetType(BodyId))
    {
    case b2_staticBody:    return StaticBodyColor.ToFColor(true);
    case b2_dynamicBody:   return DynamicBodyColor.ToFColor(true);
    case b2_kinematicBody: return KinematicBodyColor.ToFColor(true);
    default:               return FColor::White;
    }
}

// ==================== Solid Mesh ====================

void UBox2DRendererComponent::UpdateSolidMesh()
{
    b2WorldId WorldId = CachedWorldComponent->GetWorldId();
    ClearAllMeshSections();

    struct FShapeCollector
    {
        struct FPoly { b2WorldTransform Transform; TArray<b2Vec2> Verts; float Radius; b2HexColor Color; };
        struct FCirc { b2WorldTransform Transform; b2Vec2 Center; float Radius; b2HexColor Color; };
        struct FCaps { b2Pos P1; b2Pos P2; float Radius; b2HexColor Color; };
        TArray<FPoly> Polygons;
        TArray<FCirc> Circles;
        TArray<FCaps> Capsules;
    };

    FShapeCollector Collector;

    b2DebugDraw Draw = b2DefaultDebugDraw();
    Draw.drawShapes = true;
    Draw.drawJoints = false;
    Draw.drawBounds = false;
    Draw.context = &Collector;

    Draw.DrawSolidPolygonFcn = [](b2WorldTransform Transform, const b2Vec2* Verts, int VertexCount, float Radius, b2HexColor Color, void* Context)
    {
        auto* Col = static_cast<FShapeCollector*>(Context);
        FShapeCollector::FPoly Poly;
        Poly.Transform = Transform; Poly.Radius = Radius; Poly.Color = Color;
        for (int i = 0; i < VertexCount; i++) Poly.Verts.Add(Verts[i]);
        Col->Polygons.Add(MoveTemp(Poly));
    };

    Draw.DrawSolidCircleFcn = [](b2WorldTransform Transform, b2Vec2 Center, float Radius, b2HexColor Color, void* Context)
    {
        auto* Col = static_cast<FShapeCollector*>(Context);
        FShapeCollector::FCirc C;
        C.Transform = Transform; C.Center = Center; C.Radius = Radius; C.Color = Color;
        Col->Circles.Add(MoveTemp(C));
    };

    Draw.DrawSolidCapsuleFcn = [](b2Pos P1, b2Pos P2, float Radius, b2HexColor Color, void* Context)
    {
        auto* Col = static_cast<FShapeCollector*>(Context);
        FShapeCollector::FCaps Cap;
        Cap.P1 = P1; Cap.P2 = P2; Cap.Radius = Radius; Cap.Color = Color;
        Col->Capsules.Add(MoveTemp(Cap));
    };

    b2World_Draw(WorldId, &Draw);

    TArray<FVector> Vertices; TArray<int32> Indices; TArray<FVector> Normals;
    TArray<FVector2D> UVs; TArray<FColor> Colors;
    int32 BaseIndex = 0; float CurrentDepth = 0.0f;

    for (const auto& Poly : Collector.Polygons)
    {
        TArray<b2Vec2> WV;
        for (const auto& V : Poly.Verts) WV.Add(b2TransformPoint(Poly.Transform, V));
        GeneratePolygonVertices(WV.GetData(), WV.Num(), CurrentDepth, Vertices, Indices, Normals, UVs, Colors, BaseIndex);
        CurrentDepth += DepthOffset;
    }

    for (const auto& Cir : Collector.Circles)
    {
        b2Vec2 WC = b2TransformPoint(Cir.Transform, Cir.Center);
        GenerateCircleVertices(WC, Cir.Radius, CurrentDepth, Vertices, Indices, Normals, UVs, Colors, BaseIndex);
        CurrentDepth += DepthOffset;
    }

    for (const auto& Cap : Collector.Capsules)
    {
        GenerateCapsuleVertices(
            { (float)Cap.P1.x, (float)Cap.P1.y }, { (float)Cap.P2.x, (float)Cap.P2.y },
            Cap.Radius, CurrentDepth, Vertices, Indices, Normals, UVs, Colors, BaseIndex);
        CurrentDepth += DepthOffset;
    }

    if (Vertices.Num() > 0)
    {
        EnsureMaterial();
        SetMaterial(0, DefaultMaterial);
        CreateMeshSection(0, Vertices, Indices, Normals, UVs, Colors, TArray<FProcMeshTangent>(), false);
    }
}

// ==================== Debug Draw ====================

void UBox2DRendererComponent::UpdateDebugDraw()
{
    UWorld* World = GetWorld();
    if (!World) return;

    b2DebugDraw Draw = b2DefaultDebugDraw();
    SetupDebugDraw(Draw);
    b2World_Draw(CachedWorldComponent->GetWorldId(), &Draw);
}

void UBox2DRendererComponent::SetupDebugDraw(b2DebugDraw& Draw)
{
    Draw.drawShapes = bDrawShapes;
    Draw.drawJoints = bDrawJoints;
    Draw.drawBounds = bDrawBounds;
    Draw.drawContacts = bDrawContactPoints;
    Draw.drawContactNormals = bDrawContactNormals;
    Draw.jointScale = JointDrawScale;
    Draw.context = this;

    Draw.DrawLineFcn = [](b2Pos P1, b2Pos P2, b2HexColor Color, void* Context)
    {
        auto* S = static_cast<UBox2DRendererComponent*>(Context);
        if (UWorld* W = S->GetWorld())
            DrawDebugLine(W, S->B2PosToUE(P1), S->B2PosToUE(P2), HexColorToFColor(Color), false, -1.0f, 0, S->DebugLineThickness);
    };

    Draw.DrawPolygonFcn = [](b2WorldTransform T, const b2Vec2* V, int N, b2HexColor Color, void* Context)
    {
        // Used by joints (e.g. weld joint) — draw as simple cross lines instead of billboard polygon
        auto* S = static_cast<UBox2DRendererComponent*>(Context);
        if (UWorld* W = S->GetWorld())
        {
            FColor C = HexColorToFColor(Color);
            b2Vec2 Center = {0, 0};
            for (int i = 0; i < N; i++) { Center.x += V[i].x; Center.y += V[i].y; }
            Center.x /= N; Center.y /= N;
            b2Vec2 WC = b2TransformPoint(T, Center);
            b2Vec2 V0 = b2TransformPoint(T, V[0]);
            b2Vec2 V2 = b2TransformPoint(T, V[N / 2]);
            DrawDebugLine(W, S->B2ToUE(WC), S->B2ToUE(V0), C, false, -1.0f, 0, S->DebugLineThickness);
            DrawDebugLine(W, S->B2ToUE(WC), S->B2ToUE(V2), C, false, -1.0f, 0, S->DebugLineThickness);
        }
    };

    Draw.DrawCircleFcn = [](b2Pos Center, float Radius, b2HexColor Color, void* Context)
    {
        // Used by joints (e.g. revolute joint) — draw as cross lines instead of billboard circle
        auto* S = static_cast<UBox2DRendererComponent*>(Context);
        if (UWorld* W = S->GetWorld())
        {
            FColor C = HexColorToFColor(Color);
            FVector UC = S->B2PosToUE(Center);
            float R = Radius * S->PixelsPerMeter;
            DrawDebugLine(W, UC + FVector(R, 0, 0), UC + FVector(-R, 0, 0), C, false, -1.0f, 0, S->DebugLineThickness);
            DrawDebugLine(W, UC + FVector(0, 0, R), UC + FVector(0, 0, -R), C, false, -1.0f, 0, S->DebugLineThickness);
        }
    };

    Draw.DrawPointFcn = [](b2Pos P, float Size, b2HexColor Color, void* Context)
    {
        // DrawDebugPoint renders a billboard sprite — use cross lines instead
        auto* S = static_cast<UBox2DRendererComponent*>(Context);
        if (UWorld* W = S->GetWorld())
        {
            FColor C = HexColorToFColor(Color);
            FVector UP = S->B2PosToUE(P);
            float R = Size * 0.5f * S->PixelsPerMeter;
            DrawDebugLine(W, UP + FVector(R, 0, 0), UP + FVector(-R, 0, 0), C, false, -1.0f, 0, S->DebugLineThickness);
            DrawDebugLine(W, UP + FVector(0, 0, R), UP + FVector(0, 0, -R), C, false, -1.0f, 0, S->DebugLineThickness);
        }
    };

    // Solid callbacks are used by shapes (not joints) — draw full outlines
    Draw.DrawSolidPolygonFcn = [](b2WorldTransform T, const b2Vec2* V, int N, float Radius, b2HexColor Color, void* Context)
    {
        auto* S = static_cast<UBox2DRendererComponent*>(Context);
        if (UWorld* W = S->GetWorld())
        {
            FColor C = HexColorToFColor(Color);
            for (int i = 0; i < N; i++)
            {
                b2Vec2 A = b2TransformPoint(T, V[i]);
                b2Vec2 B = b2TransformPoint(T, V[(i + 1) % N]);
                DrawDebugLine(W, S->B2ToUE(A), S->B2ToUE(B), C, false, -1.0f, 0, S->DebugLineThickness);
            }
        }
    };

    Draw.DrawSolidCircleFcn = [](b2WorldTransform T, b2Vec2 Center, float Radius, b2HexColor Color, void* Context)
    {
        auto* S = static_cast<UBox2DRendererComponent*>(Context);
        if (UWorld* W = S->GetWorld())
        {
            FColor C = HexColorToFColor(Color);
            b2Vec2 WC = b2TransformPoint(T, Center);
            FVector UC = S->B2ToUE(WC);
            float R = Radius * S->PixelsPerMeter;
            int32 Segs = S->CircleSegments;
            float Step = 2.0f * PI / Segs;
            for (int32 i = 0; i < Segs; i++)
            {
                float A1 = i * Step;
                float A2 = (i + 1) * Step;
                DrawDebugLine(W,
                    UC + FVector(FMath::Cos(A1) * R, 0, FMath::Sin(A1) * R),
                    UC + FVector(FMath::Cos(A2) * R, 0, FMath::Sin(A2) * R),
                    C, false, -1.0f, 0, S->DebugLineThickness);
            }
        }
    };

    Draw.DrawSolidCapsuleFcn = [](b2Pos P1, b2Pos P2, float Radius, b2HexColor Color, void* Context)
    {
        auto* S = static_cast<UBox2DRendererComponent*>(Context);
        if (UWorld* W = S->GetWorld())
        {
            FColor C = HexColorToFColor(Color);
            FVector UP1 = S->B2PosToUE(P1);
            FVector UP2 = S->B2PosToUE(P2);
            float UR = Radius * S->PixelsPerMeter;
            DrawDebugLine(W, UP1, UP2, C, false, -1.0f, 0, S->DebugLineThickness);

            // Draw semicircle caps
            int32 Segs = S->CircleSegments;
            b2Vec2 Dir = { (float)(P2.x - P1.x), (float)(P2.y - P1.y) };
            float Len = b2Length(Dir);
            if (Len > 1e-6f)
            {
                Dir.x /= Len; Dir.y /= Len;
                float BaseAngle = FMath::Atan2(Dir.y, Dir.x);
                float Step = PI / (Segs / 2);
                for (int32 Cap = 0; Cap < 2; Cap++)
                {
                    FVector CapCenter = (Cap == 0) ? UP1 : UP2;
                    float StartAngle = (Cap == 0) ? BaseAngle + PI : BaseAngle;
                    for (int32 i = 0; i < Segs / 2; i++)
                    {
                        float A1 = StartAngle + i * Step;
                        float A2 = StartAngle + (i + 1) * Step;
                        DrawDebugLine(W,
                            CapCenter + FVector(FMath::Cos(A1) * UR, 0, FMath::Sin(A1) * UR),
                            CapCenter + FVector(FMath::Cos(A2) * UR, 0, FMath::Sin(A2) * UR),
                            C, false, -1.0f, 0, S->DebugLineThickness);
                    }
                }
            }
        }
    };
}

// ==================== Geometry Generation ====================

void UBox2DRendererComponent::GenerateCircleVertices(const b2Vec2& Center, float Radius, float Depth,
    TArray<FVector>& OV, TArray<int32>& OI, TArray<FVector>& ON,
    TArray<FVector2D>& OU, TArray<FColor>& OC, int32& BI)
{
    FColor Col = FColor(200, 100, 100, 255);
    FVector UC = B2ToUE(Center, Depth);
    float UR = Radius * PixelsPerMeter;

    OV.Add(UC); ON.Add(FVector(0, -1, 0)); OU.Add(FVector2D(0.5f, 0.5f)); OC.Add(Col);

    float Step = 2.0f * PI / CircleSegments;
    for (int32 i = 0; i < CircleSegments; i++)
    {
        float A = i * Step;
        OV.Add(UC + FVector(FMath::Cos(A) * UR, 0, FMath::Sin(A) * UR));
        ON.Add(FVector(0, -1, 0));
        OU.Add(FVector2D(0.5f + 0.5f * FMath::Cos(A), 0.5f + 0.5f * FMath::Sin(A)));
        OC.Add(Col);
    }
    for (int32 i = 0; i < CircleSegments; i++)
    {
        OI.Add(BI); OI.Add(BI + 1 + i); OI.Add(BI + 1 + (i + 1) % CircleSegments);
    }
    BI += 1 + CircleSegments;
}

void UBox2DRendererComponent::GeneratePolygonVertices(const b2Vec2* V, int32 N, float Depth,
    TArray<FVector>& OV, TArray<int32>& OI, TArray<FVector>& ON,
    TArray<FVector2D>& OU, TArray<FColor>& OC, int32& BI)
{
    FColor Col = FColor(100, 200, 100, 255);
    b2Vec2 C = {0, 0};
    for (int32 i = 0; i < N; i++) { C.x += V[i].x; C.y += V[i].y; }
    C.x /= N; C.y /= N;

    for (int32 i = 0; i < N; i++)
    {
        OV.Add(B2ToUE(V[i], Depth));
        ON.Add(FVector(0, -1, 0));
        OU.Add(FVector2D((V[i].x - C.x) * 0.5f + 0.5f, (V[i].y - C.y) * 0.5f + 0.5f));
        OC.Add(Col);
    }
    for (int32 i = 1; i < N - 1; i++)
    {
        OI.Add(BI); OI.Add(BI + i); OI.Add(BI + i + 1);
    }
    BI += N;
}

void UBox2DRendererComponent::GenerateCapsuleVertices(const b2Vec2& P1, const b2Vec2& P2, float Radius, float Depth,
    TArray<FVector>& OV, TArray<int32>& OI, TArray<FVector>& ON,
    TArray<FVector2D>& OU, TArray<FColor>& OC, int32& BI)
{
    FColor Col = FColor(100, 100, 200, 255);
    b2Vec2 D = { P2.x - P1.x, P2.y - P1.y };
    float L = b2Length(D);
    if (L < 1e-6f) return;
    D.x /= L; D.y /= L;
    b2Vec2 P = { -D.y, D.x };

    b2Vec2 Corners[4] = {
        { P1.x + P.x * Radius, P1.y + P.y * Radius },
        { P1.x - P.x * Radius, P1.y - P.y * Radius },
        { P2.x - P.x * Radius, P2.y - P.y * Radius },
        { P2.x + P.x * Radius, P2.y + P.y * Radius }
    };
    GeneratePolygonVertices(Corners, 4, Depth, OV, OI, ON, OU, OC, BI);

    int32 HS = CircleSegments / 2;
    float Step = PI / HS;

    for (int Cap = 0; Cap < 2; Cap++)
    {
        b2Vec2 CC = (Cap == 0) ? P1 : P2;
        float BA = (Cap == 0) ? FMath::Atan2(-D.y, -D.x) : FMath::Atan2(D.y, D.x);

        OV.Add(B2ToUE(CC, Depth)); ON.Add(FVector(0, -1, 0)); OU.Add(FVector2D(0.5f, 0.5f)); OC.Add(Col);
        int32 CI = BI; BI++;

        for (int32 i = 0; i <= HS; i++)
        {
            float A = BA + PI * 0.5f + i * Step;
            b2Vec2 VV = { CC.x + FMath::Cos(A) * Radius, CC.y + FMath::Sin(A) * Radius };
            OV.Add(B2ToUE(VV, Depth)); ON.Add(FVector(0, -1, 0));
            OU.Add(FVector2D(0.5f + 0.5f * FMath::Cos(A), 0.5f + 0.5f * FMath::Sin(A)));
            OC.Add(Col);
        }
        for (int32 i = 0; i < HS; i++)
        {
            OI.Add(CI); OI.Add(CI + 1 + i); OI.Add(CI + 2 + i);
        }
        BI += HS + 1;
    }
}

void UBox2DRendererComponent::GenerateSegmentVertices(const b2Vec2& P1, const b2Vec2& P2, float Depth,
    TArray<FVector>& OV, TArray<int32>& OI, TArray<FVector>& ON,
    TArray<FVector2D>& OU, TArray<FColor>& OC, int32& BI)
{
    b2Vec2 D = { P2.x - P1.x, P2.y - P1.y };
    float L = b2Length(D);
    if (L < 1e-6f) return;
    float TW = 0.02f;
    D.x /= L; D.y /= L;
    b2Vec2 P = { -D.y * TW, D.x * TW };

    b2Vec2 Corners[4] = {
        { P1.x + P.x, P1.y + P.y }, { P1.x - P.x, P1.y - P.y },
        { P2.x - P.x, P2.y - P.y }, { P2.x + P.x, P2.y + P.y }
    };
    GeneratePolygonVertices(Corners, 4, Depth, OV, OI, ON, OU, OC, BI);
}

#undef LOCTEXT_NAMESPACE

void UBox2DRendererComponent::EnsureMaterial()
{
    if (DefaultMaterial) return;

    if (CustomMaterial)
    {
        DefaultMaterial = UMaterialInstanceDynamic::Create(CustomMaterial, this);
        return;
    }

    // Use the engine's debug unlit material which supports vertex colors,
    // or fall back to BasicShapeMaterial
    UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/EngineDebugMaterials/UnlitMaterial"));
    if (!BaseMat)
    {
        BaseMat = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    }
    if (BaseMat)
    {
        DefaultMaterial = UMaterialInstanceDynamic::Create(BaseMat, this);
    }
}
