#pragma once

#include "Modules/ModuleInterface.h"

#define BOX2D_COLLISION_EDITOR_MODULE_NAME "Box2DCollisionEditor"

class IBox2DCollisionEditorModule : public IModuleInterface
{
public:
    virtual uint32 GetBox2DAssetCategory() const { return 0; }
};
