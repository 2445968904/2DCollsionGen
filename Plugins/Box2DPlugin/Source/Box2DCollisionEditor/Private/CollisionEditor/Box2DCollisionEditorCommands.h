#pragma once

#include "Framework/Commands/Commands.h"
#include "Box2DStyle.h"
#include "Styling/ISlateStyle.h"

class FBox2DCollisionEditorCommands : public TCommands<FBox2DCollisionEditorCommands>
{
public:
    FBox2DCollisionEditorCommands()
        : TCommands<FBox2DCollisionEditorCommands>(
            TEXT("Box2DCollisionEditor"),
            NSLOCTEXT("Contexts", "Box2DCollisionEditor", "Box2D Collision Editor"),
            NAME_None,
            FBox2DStyle::Get()->GetStyleSetName())
    {
    }

    virtual void RegisterCommands() override;

    // Editing modes
    TSharedPtr<FUICommandInfo> EnterViewMode;
    TSharedPtr<FUICommandInfo> EnterEditShapesMode;
    TSharedPtr<FUICommandInfo> EnterEditJointsMode;

    // Show toggles
    TSharedPtr<FUICommandInfo> SetShowGrid;
    TSharedPtr<FUICommandInfo> SetShowBounds;
    TSharedPtr<FUICommandInfo> SetShowCollision;
    TSharedPtr<FUICommandInfo> SetShowJoints;
};
