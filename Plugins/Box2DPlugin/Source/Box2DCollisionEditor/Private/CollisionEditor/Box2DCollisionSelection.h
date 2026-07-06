#pragma once

#include "Math/Vector2D.h"

// Selection identifier types for the collision editor
struct FBox2DCollisionSelectionTypes
{
    static const FName Shape;
    static const FName Vertex;
    static const FName Body;
};

// Base class for selectable items in the collision editor
class FBox2DSelectedItem
{
public:
    virtual ~FBox2DSelectedItem() = default;

    // Get the type ID for casting
    virtual FName GetTypeId() const = 0;

    // Get the world-space 3D position for widget placement (XZ plane)
    virtual FVector GetWorldPos() const = 0;

    // Apply a delta to this item
    virtual void ApplyDelta(const FVector2D& InDrag, const FRotator& InRot, const FVector& InScale) = 0;

    // Try to cast to a specific type
    template<typename T>
    const T* CastTo(FName TypeId) const
    {
        return (GetTypeId() == TypeId) ? static_cast<const T*>(this) : nullptr;
    }
};

// A selected collision shape (whole shape)
class FBox2DSelectedShape : public FBox2DSelectedItem
{
public:
    int32 BodyIndex;
    int32 ShapeIndex;
    class UBox2DCollisionProfile* Profile;

    FBox2DSelectedShape() : BodyIndex(INDEX_NONE), ShapeIndex(INDEX_NONE), Profile(nullptr) {}
    FBox2DSelectedShape(int32 InBodyIndex, int32 InShapeIndex, class UBox2DCollisionProfile* InProfile)
        : BodyIndex(InBodyIndex), ShapeIndex(InShapeIndex), Profile(InProfile) {}

    virtual FName GetTypeId() const override { return FBox2DCollisionSelectionTypes::Shape; }
    virtual FVector GetWorldPos() const override;
    virtual void ApplyDelta(const FVector2D& InDrag, const FRotator& InRot, const FVector& InScale) override;
};

// A selected vertex of a polygon shape
class FBox2DSelectedVertex : public FBox2DSelectedItem
{
public:
    int32 BodyIndex;
    int32 ShapeIndex;
    int32 VertexIndex;
    class UBox2DCollisionProfile* Profile;

    FBox2DSelectedVertex() : BodyIndex(INDEX_NONE), ShapeIndex(INDEX_NONE), VertexIndex(INDEX_NONE), Profile(nullptr) {}
    FBox2DSelectedVertex(int32 InBodyIndex, int32 InShapeIndex, int32 InVertexIndex, class UBox2DCollisionProfile* InProfile)
        : BodyIndex(InBodyIndex), ShapeIndex(InShapeIndex), VertexIndex(InVertexIndex), Profile(InProfile) {}

    virtual FName GetTypeId() const override { return FBox2DCollisionSelectionTypes::Vertex; }
    virtual FVector GetWorldPos() const override;
    virtual void ApplyDelta(const FVector2D& InDrag, const FRotator& InRot, const FVector& InScale) override;
};

// A selected body origin
class FBox2DSelectedBody : public FBox2DSelectedItem
{
public:
    int32 BodyIndex;
    class UBox2DCollisionProfile* Profile;

    FBox2DSelectedBody() : BodyIndex(INDEX_NONE), Profile(nullptr) {}
    FBox2DSelectedBody(int32 InBodyIndex, class UBox2DCollisionProfile* InProfile)
        : BodyIndex(InBodyIndex), Profile(InProfile) {}

    virtual FName GetTypeId() const override { return FBox2DCollisionSelectionTypes::Body; }
    virtual FVector GetWorldPos() const override;
    virtual void ApplyDelta(const FVector2D& InDrag, const FRotator& InRot, const FVector& InScale) override;
};
