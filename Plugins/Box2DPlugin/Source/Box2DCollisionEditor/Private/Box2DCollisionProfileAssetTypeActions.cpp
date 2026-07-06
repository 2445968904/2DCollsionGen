#include "Box2DCollisionEditorPCH.h"
#include "Box2DCollisionProfileAssetTypeActions.h"
#include "Box2DCollisionProfile.h"
#include "ToolMenuSection.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FBox2DCollisionProfileAssetTypeActions::FBox2DCollisionProfileAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
    : MyAssetCategory(InAssetCategory)
{
}

FText FBox2DCollisionProfileAssetTypeActions::GetName() const
{
    return LOCTEXT("FBox2DCollisionProfileAssetTypeActionsName", "Box2D Collision Profile");
}

FColor FBox2DCollisionProfileAssetTypeActions::GetTypeColor() const
{
    return FColor(0, 200, 200);
}

UClass* FBox2DCollisionProfileAssetTypeActions::GetSupportedClass() const
{
    return UBox2DCollisionProfile::StaticClass();
}

void FBox2DCollisionProfileAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
    // For now, open in default property editor. Phase 3 will add custom editor.
    //const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
    // Custom editor will be wired here in Phase 3.
}

uint32 FBox2DCollisionProfileAssetTypeActions::GetCategories()
{
    return MyAssetCategory;
}

void FBox2DCollisionProfileAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
    auto Profiles = GetTypedWeakObjectPtrs<UBox2DCollisionProfile>(InObjects);

    Section.AddMenuEntry(
        "Box2DProfile_AddBody",
        LOCTEXT("Box2DProfile_AddBody", "Add Default Body"),
        LOCTEXT("Box2DProfile_AddBodyTooltip", "Adds a default dynamic body with a box shape to the selected profiles."),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus"),
        FUIAction(
            FExecuteAction::CreateSP(this, &FBox2DCollisionProfileAssetTypeActions::ExecuteAddBody, Profiles),
            FCanExecuteAction()
        )
    );

    Section.AddMenuEntry(
        "Box2DProfile_ClearAll",
        LOCTEXT("Box2DProfile_ClearAll", "Clear All Bodies and Joints"),
        LOCTEXT("Box2DProfile_ClearAllTooltip", "Removes all bodies and joints from the selected profiles."),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Delete"),
        FUIAction(
            FExecuteAction::CreateSP(this, &FBox2DCollisionProfileAssetTypeActions::ExecuteClearAll, Profiles),
            FCanExecuteAction()
        )
    );
}

void FBox2DCollisionProfileAssetTypeActions::ExecuteAddBody(TArray<TWeakObjectPtr<UBox2DCollisionProfile>> Objects)
{
    for (auto& WeakProfile : Objects)
    {
        if (UBox2DCollisionProfile* Profile = WeakProfile.Get())
        {
            Profile->Modify();
            Profile->AddDefaultBody();
        }
    }
}

void FBox2DCollisionProfileAssetTypeActions::ExecuteClearAll(TArray<TWeakObjectPtr<UBox2DCollisionProfile>> Objects)
{
    for (auto& WeakProfile : Objects)
    {
        if (UBox2DCollisionProfile* Profile = WeakProfile.Get())
        {
            Profile->Modify();
            Profile->Bodies.Empty();
            Profile->Joints.Empty();
        }
    }
}

#undef LOCTEXT_NAMESPACE
