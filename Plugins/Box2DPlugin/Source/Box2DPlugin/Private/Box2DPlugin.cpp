#include "Box2DPluginPCH.h"
#include "Box2DPlugin.h"
#include "Box2DWorldComponent.h"
#include "Box2DRendererComponent.h"
#include "box2d/box2d.h"

DEFINE_LOG_CATEGORY(Box2DLog);

void FBox2DPlugin::StartupModule()
{
    UE_LOG(Box2DLog, Log, TEXT("Box2DPlugin module started"));
}

void FBox2DPlugin::ShutdownModule()
{
    UE_LOG(Box2DLog, Log, TEXT("Box2DPlugin module shutdown"));
}

IMPLEMENT_MODULE(FBox2DPlugin, Box2DPlugin)
