#include "Box2DCollisionEditorPCH.h"
#include "Box2DStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyle.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

TSharedPtr<FSlateStyleSet> FBox2DStyle::StyleSet = nullptr;
TSharedPtr<class ISlateStyle> FBox2DStyle::Get() { return StyleSet; }

FName FBox2DStyle::GetStyleSetName()
{
    static FName Box2DStyleName(TEXT("Box2DStyle"));
    return Box2DStyleName;
}

void FBox2DStyle::Initialize()
{
    if (StyleSet.IsValid())
    {
        return;
    }

    const FVector2D Icon16x16(16.0f, 16.0f);
    const FVector2D Icon40x40(40.0f, 40.0f);
    const FVector2D Icon64x64(64.0f, 64.0f);

    StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
    StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

    // Collision profile class icons/thumbnails (using engine icons as placeholders)
    StyleSet->Set("ClassIcon.Box2DCollisionProfile", new IMAGE_BRUSH(TEXT("Icons/icon_StaticMesh_16x"), Icon16x16));
    StyleSet->Set("ClassThumbnail.Box2DCollisionProfile", new IMAGE_BRUSH(TEXT("Icons/icon_StaticMesh_64x"), Icon64x64));

    // Collision editor toolbar icons
    StyleSet->Set("Box2DEditor.EnterViewMode", new IMAGE_BRUSH(TEXT("Icons/icon_EditorModes_StandSelect_40x"), Icon40x40));
    StyleSet->Set("Box2DEditor.EnterViewMode.Small", new IMAGE_BRUSH(TEXT("Icons/icon_EditorModes_StandSelect_40x"), FVector2D(20.0f, 20.0f)));
    StyleSet->Set("Box2DEditor.EnterEditShapesMode", new IMAGE_BRUSH(TEXT("Icons/icon_StaticMeshEd_Collision_40x"), Icon40x40));
    StyleSet->Set("Box2DEditor.EnterEditShapesMode.Small", new IMAGE_BRUSH(TEXT("Icons/icon_StaticMeshEd_Collision_40x"), FVector2D(20.0f, 20.0f)));
    StyleSet->Set("Box2DEditor.EnterEditJointsMode", new IMAGE_BRUSH(TEXT("Icons/icon_Physics_40x"), Icon40x40));
    StyleSet->Set("Box2DEditor.EnterEditJointsMode.Small", new IMAGE_BRUSH(TEXT("Icons/icon_Physics_40x"), FVector2D(20.0f, 20.0f)));

    // Shape editing icons (using engine placeholders)
    StyleSet->Set("Box2DEditor.AddBoxShape", new IMAGE_BRUSH(TEXT("Icons/icon_StaticMeshEd_Collision_40x"), Icon40x40));
    StyleSet->Set("Box2DEditor.AddBoxShape.Small", new IMAGE_BRUSH(TEXT("Icons/icon_StaticMeshEd_Collision_40x"), FVector2D(20.0f, 20.0f)));
    StyleSet->Set("Box2DEditor.AddCircleShape", new IMAGE_BRUSH(TEXT("Icons/icon_MatEd_Sphere_40x"), Icon40x40));
    StyleSet->Set("Box2DEditor.AddCircleShape.Small", new IMAGE_BRUSH(TEXT("Icons/icon_MatEd_Sphere_40x"), FVector2D(20.0f, 20.0f)));

    FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

#undef IMAGE_BRUSH

void FBox2DStyle::Shutdown()
{
    if (StyleSet.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
        ensure(StyleSet.IsUnique());
        StyleSet.Reset();
    }
}
