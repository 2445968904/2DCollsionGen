#include "Box2DCollisionEditorPCH.h"
#include "Box2DCollisionEditorModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetDataTagMap.h"

#include "Box2DCollisionProfileAssetTypeActions.h"
#include "Box2DCollisionProfileFactory.h"
#include "Box2DStyle.h"

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
    }

    virtual void ShutdownModule() override
    {
        FCoreDelegates::GetOnPostEngineInit().RemoveAll(this);

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
