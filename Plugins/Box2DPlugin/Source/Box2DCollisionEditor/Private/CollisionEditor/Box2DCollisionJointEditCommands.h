#pragma once

#include "Framework/Commands/Commands.h"
#include "Box2DStyle.h"
#include "Styling/ISlateStyle.h"

class FBox2DCollisionJointEditCommands : public TCommands<FBox2DCollisionJointEditCommands>
{
public:
    FBox2DCollisionJointEditCommands()
        : TCommands<FBox2DCollisionJointEditCommands>(
            TEXT("Box2DCollisionJointEdit"),
            NSLOCTEXT("Contexts", "Box2DCollisionJointEdit", "Box2D Collision Joint Edit"),
            NAME_None,
            FBox2DStyle::Get()->GetStyleSetName())
    {
    }

    virtual void RegisterCommands() override;

    TSharedPtr<FUICommandInfo> AddDistanceJoint;
    TSharedPtr<FUICommandInfo> AddRevoluteJoint;
    TSharedPtr<FUICommandInfo> AddPrismaticJoint;
    TSharedPtr<FUICommandInfo> AddWeldJoint;
    TSharedPtr<FUICommandInfo> DeleteJoint;
};
