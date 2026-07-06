#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionSelection.h"
#include "CollisionEditor/Box2DEditorUtils.h"
#include "Box2DCollisionProfile.h"
#include "Box2DCollisionTypes.h"

using namespace Box2DEditorUtils;

const FName FBox2DCollisionSelectionTypes::Shape(TEXT("Box2DShape"));
const FName FBox2DCollisionSelectionTypes::Vertex(TEXT("Box2DVertex"));
const FName FBox2DCollisionSelectionTypes::Body(TEXT("Box2DBody"));

FVector FBox2DSelectedShape::GetWorldPos() const
{
    if (!Profile || !Profile->Bodies.IsValidIndex(BodyIndex)) return FVector::ZeroVector;
    const FBox2DCollisionBody& Body = Profile->Bodies[BodyIndex];
    if (!Body.Shapes.IsValidIndex(ShapeIndex)) return ToXZPlane(Body.Position * Profile->PixelsPerMeter);
    const FBox2DCollisionShape& Shape = Body.Shapes[ShapeIndex];
    return ToXZPlane((Body.Position + Shape.Center) * Profile->PixelsPerMeter);
}

void FBox2DSelectedShape::ApplyDelta(const FVector2D& InDrag, const FRotator& InRot, const FVector& InScale)
{
    if (!Profile || !Profile->Bodies.IsValidIndex(BodyIndex)) return;
    if (!Profile->Bodies[BodyIndex].Shapes.IsValidIndex(ShapeIndex)) return;

    FBox2DCollisionShape& Shape = Profile->Bodies[BodyIndex].Shapes[ShapeIndex];
    float InvScale = 1.0f / Profile->PixelsPerMeter;

    // Move shape center
    Shape.Center += InDrag * InvScale;

    // Apply rotation
    if (!InRot.IsNearlyZero())
    {
        Shape.Rotation += InRot.Yaw;
    }
}

FVector FBox2DSelectedVertex::GetWorldPos() const
{
    if (!Profile || !Profile->Bodies.IsValidIndex(BodyIndex)) return FVector::ZeroVector;
    const FBox2DCollisionBody& Body = Profile->Bodies[BodyIndex];
    if (!Body.Shapes.IsValidIndex(ShapeIndex)) return ToXZPlane(Body.Position * Profile->PixelsPerMeter);
    const FBox2DCollisionShape& Shape = Body.Shapes[ShapeIndex];

    if (Shape.ShapeType == EBox2DCollisionShapeType::Polygon && Shape.Vertices.IsValidIndex(VertexIndex))
    {
        return ToXZPlane((Body.Position + Shape.Center + Shape.Vertices[VertexIndex]) * Profile->PixelsPerMeter);
    }
    // For box/circle, return center
    return ToXZPlane((Body.Position + Shape.Center) * Profile->PixelsPerMeter);
}

void FBox2DSelectedVertex::ApplyDelta(const FVector2D& InDrag, const FRotator& InRot, const FVector& InScale)
{
    if (!Profile || !Profile->Bodies.IsValidIndex(BodyIndex)) return;
    FBox2DCollisionBody& Body = Profile->Bodies[BodyIndex];
    if (!Body.Shapes.IsValidIndex(ShapeIndex)) return;
    FBox2DCollisionShape& Shape = Body.Shapes[ShapeIndex];

    float InvScale = 1.0f / Profile->PixelsPerMeter;

    if (Shape.ShapeType == EBox2DCollisionShapeType::Polygon && Shape.Vertices.IsValidIndex(VertexIndex))
    {
        Shape.Vertices[VertexIndex] += InDrag * InvScale;
    }
    else if (Shape.ShapeType == EBox2DCollisionShapeType::Box)
    {
        // For box, dragging moves the center
        Shape.Center += InDrag * InvScale;
    }
    else if (Shape.ShapeType == EBox2DCollisionShapeType::Circle)
    {
        // For circle, dragging moves the center
        Shape.Center += InDrag * InvScale;
    }
}

FVector FBox2DSelectedBody::GetWorldPos() const
{
    if (!Profile || !Profile->Bodies.IsValidIndex(BodyIndex)) return FVector::ZeroVector;
    return ToXZPlane(Profile->Bodies[BodyIndex].Position * Profile->PixelsPerMeter);
}

void FBox2DSelectedBody::ApplyDelta(const FVector2D& InDrag, const FRotator& InRot, const FVector& InScale)
{
    if (!Profile || !Profile->Bodies.IsValidIndex(BodyIndex)) return;
    float InvScale = 1.0f / Profile->PixelsPerMeter;
    Profile->Bodies[BodyIndex].Position += InDrag * InvScale;
}
