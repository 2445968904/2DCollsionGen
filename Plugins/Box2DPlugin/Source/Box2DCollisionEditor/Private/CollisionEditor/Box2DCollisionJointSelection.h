#pragma once

#include "CoreMinimal.h"
#include "Box2DCollisionTypes.h"
#include "Box2DCollisionProfile.h"

class UBox2DCollisionProfile;

// Selection types for joint editing
namespace FBox2DJointSelectionTypes
{
    extern const FName Joint;
    extern const FName AnchorA;
    extern const FName AnchorB;
}

// Base class for joint selection items
class FBox2DSelectedJointItem
{
public:
    virtual ~FBox2DSelectedJointItem() = default;
    virtual FName GetTypeName() const = 0;
    virtual FVector GetWorldPos(UBox2DCollisionProfile* Profile) const = 0;
    virtual void ApplyDelta(UBox2DCollisionProfile* Profile, const FVector2D& Drag2D) = 0;

    int32 JointIndex = INDEX_NONE;

    template<typename T>
    const T* CastTo(FName TypeName) const
    {
        return (GetTypeName() == TypeName) ? static_cast<const T*>(this) : nullptr;
    }
};

// Selected entire joint
class FBox2DSelectedJoint : public FBox2DSelectedJointItem
{
public:
    FBox2DSelectedJoint(int32 InJointIndex) { JointIndex = InJointIndex; }
    virtual FName GetTypeName() const override { return FBox2DJointSelectionTypes::Joint; }
    virtual FVector GetWorldPos(UBox2DCollisionProfile* Profile) const override;
    virtual void ApplyDelta(UBox2DCollisionProfile* Profile, const FVector2D& Drag2D) override;
};

// Selected anchor A of a joint
class FBox2DSelectedAnchorA : public FBox2DSelectedJointItem
{
public:
    FBox2DSelectedAnchorA(int32 InJointIndex) { JointIndex = InJointIndex; }
    virtual FName GetTypeName() const override { return FBox2DJointSelectionTypes::AnchorA; }
    virtual FVector GetWorldPos(UBox2DCollisionProfile* Profile) const override;
    virtual void ApplyDelta(UBox2DCollisionProfile* Profile, const FVector2D& Drag2D) override;
};

// Selected anchor B of a joint
class FBox2DSelectedAnchorB : public FBox2DSelectedJointItem
{
public:
    FBox2DSelectedAnchorB(int32 InJointIndex) { JointIndex = InJointIndex; }
    virtual FName GetTypeName() const override { return FBox2DJointSelectionTypes::AnchorB; }
    virtual FVector GetWorldPos(UBox2DCollisionProfile* Profile) const override;
    virtual void ApplyDelta(UBox2DCollisionProfile* Profile, const FVector2D& Drag2D) override;
};
