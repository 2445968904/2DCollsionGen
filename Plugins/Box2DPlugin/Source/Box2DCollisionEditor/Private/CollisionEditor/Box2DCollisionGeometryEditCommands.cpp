#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionGeometryEditCommands.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionGeometryEditCommands"

void FBox2DCollisionGeometryEditCommands::RegisterCommands()
{
    UI_COMMAND(AddBoxShape, "Add Box", "Add a new box collision shape.", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(AddCircleShape, "Add Circle", "Add a new circle collision shape.", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(AddPolygonShape, "Add Polygon", "Add a new polygon collision shape (convex hull from click points).", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(DeleteSelection, "Delete", "Delete the selected shape or vertex.", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
}

#undef LOCTEXT_NAMESPACE
