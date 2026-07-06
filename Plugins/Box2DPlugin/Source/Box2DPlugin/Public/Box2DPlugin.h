#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(Box2DLog, Log, All);

class BOX2DPLUGIN_API FBox2DPlugin : public IModuleInterface
{
public:
    static inline FBox2DPlugin& Get()
    {
        return FModuleManager::LoadModuleChecked<FBox2DPlugin>("Box2DPlugin");
    }

    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("Box2DPlugin");
    }

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
