#pragma once

#include "CoreMinimal.h"
#include "Box2DCollisionTypes.h"

class UStaticMesh;
class UBox2DCollisionProfile;

namespace Box2DProjectionUtils
{
    // Project a 3D position onto a 2D plane based on the projection axis
    FVector2D ProjectTo2D(const FVector& Pos3D, EBox2DProjectionAxis Axis);

    // Extract unique 2D points from a StaticMesh's vertex positions
    bool ExtractMeshVertices2D(UStaticMesh* Mesh, EBox2DProjectionAxis Axis, TArray<FVector2D>& OutPoints);

    // Compute a 2D convex hull from a set of points (Graham scan)
    // Returns vertices in counter-clockwise order
    void ComputeConvexHull(TArray<FVector2D>& InOutPoints);

    // Simplify a convex hull to at most MaxVertices by removing least-significant vertices
    void SimplifyHull(TArray<FVector2D>& InOutPoints, int32 MaxVertices = 8);

    // Compute the bounding circle of a set of 2D points
    void ComputeBoundingCircle(const TArray<FVector2D>& Points, FVector2D& OutCenter, float& OutRadius);

    // Compute the bounding box of a set of 2D points
    void ComputeBoundingBox(const TArray<FVector2D>& Points, FVector2D& OutCenter, FVector2D& OutHalfExtents);

    // Auto-generate collision shapes from a StaticMesh
    // Methods: SingleBox, SingleCircle, ConvexPolygon, DecomposedConvex
    enum class EAutoGenMethod : uint8
    {
        SingleBox,
        SingleCircle,
        ConvexPolygon,
        DecomposedConvex
    };

    // Generate collision and add to the profile's first body (creates one if empty)
    bool AutoGenerateCollision(UBox2DCollisionProfile* Profile, UStaticMesh* SourceMesh, EAutoGenMethod Method = EAutoGenMethod::ConvexPolygon);
}
