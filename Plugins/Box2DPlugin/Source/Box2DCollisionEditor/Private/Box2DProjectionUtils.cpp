#include "Box2DCollisionEditorPCH.h"
#include "Box2DProjectionUtils.h"
#include "Box2DCollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Rendering/PositionVertexBuffer.h"
#include "StaticMeshResources.h"

#define LOCTEXT_NAMESPACE "Box2DProjectionUtils"

namespace Box2DProjectionUtils
{

FVector2D ProjectTo2D(const FVector& Pos3D, EBox2DProjectionAxis Axis)
{
    switch (Axis)
    {
    case EBox2DProjectionAxis::XZ:
        return FVector2D(Pos3D.X, Pos3D.Z);
    case EBox2DProjectionAxis::XY:
        return FVector2D(Pos3D.X, Pos3D.Y);
    case EBox2DProjectionAxis::YZ:
        return FVector2D(Pos3D.Y, Pos3D.Z);
    default:
        return FVector2D(Pos3D.X, Pos3D.Z);
    }
}

bool ExtractMeshVertices2D(UStaticMesh* Mesh, EBox2DProjectionAxis Axis, TArray<FVector2D>& OutPoints)
{
    if (!Mesh) return false;

    const FStaticMeshRenderData* RenderData = Mesh->GetRenderData();
    if (!RenderData || RenderData->LODResources.Num() == 0) return false;

    const FStaticMeshLODResources& LOD = RenderData->LODResources[0];
    const FPositionVertexBuffer& PositionBuffer = LOD.VertexBuffers.PositionVertexBuffer;
    const uint32 NumVertices = PositionBuffer.GetNumVertices();

    if (NumVertices == 0) return false;

    // Collect unique 2D points (using a tolerance to deduplicate)
    TSet<uint32> AddedHashes;
    const float GridSnap = 0.001f;

    for (uint32 i = 0; i < NumVertices; i++)
    {
        const FVector3f& Pos = PositionBuffer.VertexPosition(i);
        FVector2D Pos2D = ProjectTo2D(FVector(Pos), Axis);

        // Snap to grid for deduplication
        float SnappedX = FMath::RoundToFloat(Pos2D.X / GridSnap) * GridSnap;
        float SnappedY = FMath::RoundToFloat(Pos2D.Y / GridSnap) * GridSnap;

        uint32 Hash = GetTypeHash(FVector2D(SnappedX, SnappedY));
        if (!AddedHashes.Contains(Hash))
        {
            AddedHashes.Add(Hash);
            OutPoints.Add(Pos2D);
        }
    }

    return OutPoints.Num() > 0;
}

// Cross product of vectors OA and OB (positive = counter-clockwise turn)
static float Cross2D(const FVector2D& O, const FVector2D& A, const FVector2D& B)
{
    return (A.X - O.X) * (B.Y - O.Y) - (A.Y - O.Y) * (B.X - O.X);
}

void ComputeConvexHull(TArray<FVector2D>& InOutPoints)
{
    int32 N = InOutPoints.Num();
    if (N < 3) return;

    // Find the lowest point (smallest Y, then smallest X)
    int32 Lowest = 0;
    for (int32 i = 1; i < N; i++)
    {
        if (InOutPoints[i].Y < InOutPoints[Lowest].Y ||
            (InOutPoints[i].Y == InOutPoints[Lowest].Y && InOutPoints[i].X < InOutPoints[Lowest].X))
        {
            Lowest = i;
        }
    }
    InOutPoints.Swap(0, Lowest);

    FVector2D Pivot = InOutPoints[0];

    // Sort by polar angle with respect to the pivot
    // Extract sub-array, sort, and put back
    TArray<FVector2D> RestPoints;
    RestPoints.Reserve(N - 1);
    for (int32 i = 1; i < N; i++)
    {
        RestPoints.Add(InOutPoints[i]);
    }
    RestPoints.Sort([Pivot](const FVector2D& A, const FVector2D& B)
    {
        float AngleA = FMath::Atan2(A.Y - Pivot.Y, A.X - Pivot.X);
        float AngleB = FMath::Atan2(B.Y - Pivot.Y, B.X - Pivot.X);
        return AngleA < AngleB;
    });
    for (int32 i = 0; i < RestPoints.Num(); i++)
    {
        InOutPoints[i + 1] = RestPoints[i];
    }

    // Graham scan
    TArray<FVector2D> Hull;
    Hull.Reserve(N);
    Hull.Add(InOutPoints[0]);

    for (int32 i = 1; i < N; i++)
    {
        while (Hull.Num() > 1 && Cross2D(Hull[Hull.Num() - 2], Hull[Hull.Num() - 1], InOutPoints[i]) <= 0.0f)
        {
            Hull.Pop();
        }
        Hull.Add(InOutPoints[i]);
    }

    InOutPoints = MoveTemp(Hull);
}

void SimplifyHull(TArray<FVector2D>& InOutPoints, int32 MaxVertices)
{
    if (InOutPoints.Num() <= MaxVertices) return;

    // Iteratively remove the vertex that adds the least area
    while (InOutPoints.Num() > MaxVertices && InOutPoints.Num() > 3)
    {
        int32 BestIdx = -1;
        float MinArea = FLT_MAX;
        int32 N = InOutPoints.Num();

        for (int32 i = 0; i < N; i++)
        {
            int32 Prev = (i - 1 + N) % N;
            int32 Next = (i + 1) % N;

            float Area = FMath::Abs(Cross2D(InOutPoints[Prev], InOutPoints[i], InOutPoints[Next]));
            if (Area < MinArea)
            {
                MinArea = Area;
                BestIdx = i;
            }
        }

        if (BestIdx >= 0)
        {
            InOutPoints.RemoveAt(BestIdx);
        }
        else
        {
            break;
        }
    }
}

void ComputeBoundingCircle(const TArray<FVector2D>& Points, FVector2D& OutCenter, float& OutRadius)
{
    if (Points.Num() == 0)
    {
        OutCenter = FVector2D::ZeroVector;
        OutRadius = 0.0f;
        return;
    }

    // Simple centroid + max distance approach
    FVector2D Centroid = FVector2D::ZeroVector;
    for (const FVector2D& P : Points)
    {
        Centroid += P;
    }
    Centroid /= (float)Points.Num();

    float MaxDist = 0.0f;
    for (const FVector2D& P : Points)
    {
        float Dist = (P - Centroid).Size();
        if (Dist > MaxDist) MaxDist = Dist;
    }

    OutCenter = Centroid;
    OutRadius = MaxDist;
}

void ComputeBoundingBox(const TArray<FVector2D>& Points, FVector2D& OutCenter, FVector2D& OutHalfExtents)
{
    if (Points.Num() == 0)
    {
        OutCenter = FVector2D::ZeroVector;
        OutHalfExtents = FVector2D::ZeroVector;
        return;
    }

    FVector2D Min(FLT_MAX, FLT_MAX);
    FVector2D Max(-FLT_MAX, -FLT_MAX);

    for (const FVector2D& P : Points)
    {
        Min.X = FMath::Min(Min.X, P.X);
        Min.Y = FMath::Min(Min.Y, P.Y);
        Max.X = FMath::Max(Max.X, P.X);
        Max.Y = FMath::Max(Max.Y, P.Y);
    }

    OutCenter = (Min + Max) * 0.5f;
    OutHalfExtents = (Max - Min) * 0.5f;
}

bool AutoGenerateCollision(UBox2DCollisionProfile* Profile, UStaticMesh* SourceMesh, EAutoGenMethod Method)
{
    if (!Profile || !SourceMesh) return false;

    TArray<FVector2D> Points2D;
    if (!ExtractMeshVertices2D(SourceMesh, Profile->ProjectionAxis, Points2D)) return false;

    // Convert from UE units (cm) to Box2D meters
    float InvScale = 1.0f / Profile->PixelsPerMeter;
    for (FVector2D& P : Points2D)
    {
        P *= InvScale;
    }

    // Ensure we have a body
    if (Profile->Bodies.Num() == 0)
    {
        Profile->AddDefaultBody();
    }
    FBox2DCollisionBody& Body = Profile->Bodies[0];
    Body.Shapes.Empty();

    switch (Method)
    {
    case EAutoGenMethod::SingleBox:
    {
        FVector2D Center, HalfExtents;
        ComputeBoundingBox(Points2D, Center, HalfExtents);

        FBox2DCollisionShape Shape;
        Shape.ShapeType = EBox2DCollisionShapeType::Box;
        Shape.Center = Center;
        Shape.HalfExtents = HalfExtents;
        Body.Shapes.Add(Shape);
        break;
    }
    case EAutoGenMethod::SingleCircle:
    {
        FVector2D Center;
        float Radius;
        ComputeBoundingCircle(Points2D, Center, Radius);

        FBox2DCollisionShape Shape;
        Shape.ShapeType = EBox2DCollisionShapeType::Circle;
        Shape.Center = Center;
        Shape.HalfExtents = FVector2D(Radius, 0.0f);
        Body.Shapes.Add(Shape);
        break;
    }
    case EAutoGenMethod::ConvexPolygon:
    {
        ComputeConvexHull(Points2D);
        SimplifyHull(Points2D, 8);

        if (Points2D.Num() < 3) return false;

        FBox2DCollisionShape Shape;
        Shape.ShapeType = EBox2DCollisionShapeType::Polygon;
        Shape.Vertices = Points2D;

        // Compute center as average of vertices
        FVector2D Center = FVector2D::ZeroVector;
        for (const FVector2D& V : Points2D) Center += V;
        Center /= (float)Points2D.Num();
        Shape.Center = Center;

        // Make vertices relative to center
        for (FVector2D& V : Shape.Vertices) V -= Center;

        Body.Shapes.Add(Shape);
        break;
    }
    case EAutoGenMethod::DecomposedConvex:
    {
        // For now, fall back to single convex polygon
        // Full decomposition would require an external library (e.g., HACD)
        ComputeConvexHull(Points2D);
        SimplifyHull(Points2D, 8);

        if (Points2D.Num() < 3) return false;

        FBox2DCollisionShape Shape;
        Shape.ShapeType = EBox2DCollisionShapeType::Polygon;
        Shape.Vertices = Points2D;

        FVector2D Center = FVector2D::ZeroVector;
        for (const FVector2D& V : Points2D) Center += V;
        Center /= (float)Points2D.Num();
        Shape.Center = Center;

        for (FVector2D& V : Shape.Vertices) V -= Center;

        Body.Shapes.Add(Shape);
        break;
    }
    }

    return true;
}

} // namespace Box2DProjectionUtils

#undef LOCTEXT_NAMESPACE
