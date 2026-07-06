#include "Box2DCollisionEditorPCH.h"
#include "Box2DCollisionEditorModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetDataTagMap.h"
#include "EditorModeRegistry.h"
#include "PropertyEditorModule.h"

#include "Box2DCollisionProfileAssetTypeActions.h"
#include "Box2DCollisionProfileFactory.h"
#include "Box2DStyle.h"
#include "CollisionEditor/Box2DCollisionGeometryEditMode.h"
#include "CollisionEditor/Box2DCollisionJointEditMode.h"
#include "Box2DCollisionProfileDetailsCustomization.h"
#include "Box2DCollisionProfile.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionEditor"

class FBox2DCollisionEditorModule : public IBox2DCollisionEditorModule
{
public:
    virtual void StartupModule() override
    {
        FCoreDelegates::GetOnPostEngineInit().AddRaw(this, &FBox2DCollisionEditorModule::OnPostEngineInit);
    }

    void OnPostEngineInit()
    {
        // Register slate style
        FBox2DStyle::Initialize();

        // Register the Box2D asset category
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        Box2DAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(
            FName(TEXT("Box2D")),
            LOCTEXT("Box2DAssetCategory", "Box2D"));

        // Register asset type actions
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FBox2DCollisionProfileAssetTypeActions(Box2DAssetCategoryBit)));

        // Register the geometry edit mode so the mode manager can instantiate it
        FEditorModeRegistry::Get().RegisterMode<FBox2DCollisionGeometryEditMode>(
            FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry,
            LOCTEXT("Box2DCollisionGeometryMode", "Box2D Collision Geometry"),
            FSlateIcon(),
            true
        );

        // Register the joint edit mode
        FEditorModeRegistry::Get().RegisterMode<FBox2DCollisionJointEditMode>(
            FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint,
            LOCTEXT("Box2DCollisionJointMode", "Box2D Collision Joint"),
            FSlateIcon(),
            true
        );

        // Register details customization for auto-generate collision
        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.RegisterCustomClassLayout(
            UBox2DCollisionProfile::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FBox2DCollisionProfileDetailsCustomization::MakeInstance)
        );
    }

    virtual void ShutdownModule() override
    {
        FCoreDelegates::GetOnPostEngineInit().RemoveAll(this);

        // Unregister details customization
        if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
        {
            FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
            PropertyModule.UnregisterCustomClassLayout(UBox2DCollisionProfile::StaticClass()->GetFName());
        }

        // Unregister the geometry edit mode
        FEditorModeRegistry::Get().UnregisterMode(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry);

        // Unregister the joint edit mode
        FEditorModeRegistry::Get().UnregisterMode(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint);

        // Unregister asset type actions
        if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
        {
            IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
            for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
            {
                AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
            }
        }
        CreatedAssetTypeActions.Empty();

        // Unregister slate style
        FBox2DStyle::Shutdown();
    }

    virtual uint32 GetBox2DAssetCategory() const override { return Box2DAssetCategoryBit; }

private:
    EAssetTypeCategories::Type Box2DAssetCategoryBit = EAssetTypeCategories::Misc;
    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;

    void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
    {
        AssetTools.RegisterAssetTypeActions(Action);
        CreatedAssetTypeActions.Add(Action);
    }
};

IMPLEMENT_MODULE(FBox2DCollisionEditorModule, Box2DCollisionEditor);

#undef LOCTEXT_NAMESPACE
