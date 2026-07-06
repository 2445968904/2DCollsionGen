#pragma once

#include "CoreMinimal.h"

namespace Box2DEditorUtils
{
    // Convert 2D position (X=right, Y=up in 2D) to 3D XZ plane (X=right, Z=up, Y=depth)
    inline FVector ToXZPlane(const FVector2D& Pos2D)
    {
        return FVector(Pos2D.X, 0.0f, Pos2D.Y);
    }

    // Convert 3D XZ plane position back to 2D
    inline FVector2D FromXZPlane(const FVector& WorldPos)
    {
        return FVector2D(WorldPos.X, WorldPos.Z);
    }
}
