#pragma once

#include "Modules/ModuleManager.h"

class FBox2DEditorPlugin : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
