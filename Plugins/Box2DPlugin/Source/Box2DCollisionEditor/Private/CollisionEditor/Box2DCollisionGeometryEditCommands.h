#pragma once

#include "Framework/Commands/Commands.h"
#include "Box2DStyle.h"
#include "Styling/ISlateStyle.h"

class FBox2DCollisionGeometryEditCommands : public TCommands<FBox2DCollisionGeometryEditCommands>
{
public:
    FBox2DCollisionGeometryEditCommands()
        : TCommands<FBox2DCollisionGeometryEditCommands>(
            TEXT("Box2DCollisionGeometryEditor"),
            NSLOCTEXT("Contexts", "Box2DCollisionGeometryEditor", "Box2D Collision Geometry Editor"),
            NAME_None,
            FBox2DStyle::Get()->GetStyleSetName())
    {
    }

    virtual void RegisterCommands() override;

    // Shape creation commands
    TSharedPtr<FUICommandInfo> AddBoxShape;
    TSharedPtr<FUICommandInfo> AddCircleShape;
    TSharedPtr<FUICommandInfo> AddPolygonShape;

    // Editing commands
    TSharedPtr<FUICommandInfo> DeleteSelection;
};
