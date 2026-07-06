#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionEditorCommands.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionEditor"

void FBox2DCollisionEditorCommands::RegisterCommands()
{
    // Editing modes
    UI_COMMAND(EnterViewMode, "View", "View the collision profile.", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(EnterEditShapesMode, "Edit Shapes", "Edit collision shapes.", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(EnterEditJointsMode, "Edit Joints", "Edit joints and constraints.", EUserInterfaceActionType::ToggleButton, FInputChord());

    // Show toggles
    UI_COMMAND(SetShowGrid, "Grid", "Displays the viewport grid.", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(SetShowBounds, "Bounds", "Toggles display of bounds.", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(SetShowCollision, "Collision", "Toggles display of collision shapes.", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(SetShowJoints, "Joints", "Toggles display of joints.", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(SetShowSourceMesh, "Source Mesh", "Toggles display of the source StaticMesh.", EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE
