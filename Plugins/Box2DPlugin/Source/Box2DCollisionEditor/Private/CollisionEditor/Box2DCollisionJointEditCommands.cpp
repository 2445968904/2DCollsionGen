#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionJointEditCommands.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionJointEdit"

void FBox2DCollisionJointEditCommands::RegisterCommands()
{
    UI_COMMAND(AddDistanceJoint, "Distance", "Add a distance joint between two bodies.", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(AddRevoluteJoint, "Revolute", "Add a revolute joint between two bodies.", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(AddPrismaticJoint, "Prismatic", "Add a prismatic joint between two bodies.", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(AddWeldJoint, "Weld", "Add a weld joint between two bodies.", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(DeleteJoint, "Delete", "Delete the selected joint or anchor.", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
}

#undef LOCTEXT_NAMESPACE
