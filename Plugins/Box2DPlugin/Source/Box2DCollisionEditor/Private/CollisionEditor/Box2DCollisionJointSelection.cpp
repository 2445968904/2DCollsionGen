#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionJointSelection.h"
#include "CollisionEditor/Box2DEditorUtils.h"

using namespace Box2DEditorUtils;

const FName FBox2DJointSelectionTypes::Joint(TEXT("Box2DJoint"));
const FName FBox2DJointSelectionTypes::AnchorA(TEXT("Box2DAnchorA"));
const FName FBox2DJointSelectionTypes::AnchorB(TEXT("Box2DAnchorB"));

FVector FBox2DSelectedJoint::GetWorldPos(UBox2DCollisionProfile* Profile) const
{
    if (!Profile || !Profile->Joints.IsValidIndex(JointIndex)) return FVector::ZeroVector;
    const FBox2DCollisionJoint& Joint = Profile->Joints[JointIndex];
    int32 BodyAIdx = Profile->FindBodyIndex(Joint.BodyAName);
    if (BodyAIdx == INDEX_NONE) return FVector::ZeroVector;
    FVector2D AnchorWorld = (Profile->Bodies[BodyAIdx].Position + Joint.AnchorA) * Profile->PixelsPerMeter;
    return ToXZPlane(AnchorWorld);
}

void FBox2DSelectedJoint::ApplyDelta(UBox2DCollisionProfile* Profile, const FVector2D& Drag2D)
{
    if (!Profile || !Profile->Joints.IsValidIndex(JointIndex)) return;
    FBox2DCollisionJoint& Joint = Profile->Joints[JointIndex];
    float InvScale = 1.0f / Profile->PixelsPerMeter;
    Joint.AnchorA += Drag2D * InvScale;
    Joint.AnchorB += Drag2D * InvScale;
}

FVector FBox2DSelectedAnchorA::GetWorldPos(UBox2DCollisionProfile* Profile) const
{
    if (!Profile || !Profile->Joints.IsValidIndex(JointIndex)) return FVector::ZeroVector;
    const FBox2DCollisionJoint& Joint = Profile->Joints[JointIndex];
    int32 BodyAIdx = Profile->FindBodyIndex(Joint.BodyAName);
    if (BodyAIdx == INDEX_NONE) return FVector::ZeroVector;
    FVector2D AnchorWorld = (Profile->Bodies[BodyAIdx].Position + Joint.AnchorA) * Profile->PixelsPerMeter;
    return ToXZPlane(AnchorWorld);
}

void FBox2DSelectedAnchorA::ApplyDelta(UBox2DCollisionProfile* Profile, const FVector2D& Drag2D)
{
    if (!Profile || !Profile->Joints.IsValidIndex(JointIndex)) return;
    float InvScale = 1.0f / Profile->PixelsPerMeter;
    Profile->Joints[JointIndex].AnchorA += Drag2D * InvScale;
}

FVector FBox2DSelectedAnchorB::GetWorldPos(UBox2DCollisionProfile* Profile) const
{
    if (!Profile || !Profile->Joints.IsValidIndex(JointIndex)) return FVector::ZeroVector;
    const FBox2DCollisionJoint& Joint = Profile->Joints[JointIndex];
    int32 BodyBIdx = Profile->FindBodyIndex(Joint.BodyBName);
    if (BodyBIdx == INDEX_NONE) return FVector::ZeroVector;
    FVector2D AnchorWorld = (Profile->Bodies[BodyBIdx].Position + Joint.AnchorB) * Profile->PixelsPerMeter;
    return ToXZPlane(AnchorWorld);
}

void FBox2DSelectedAnchorB::ApplyDelta(UBox2DCollisionProfile* Profile, const FVector2D& Drag2D)
{
    if (!Profile || !Profile->Joints.IsValidIndex(JointIndex)) return;
    float InvScale = 1.0f / Profile->PixelsPerMeter;
    Profile->Joints[JointIndex].AnchorB += Drag2D * InvScale;
}
