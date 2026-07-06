#pragma once

#include "AssetTypeActions_Base.h"

class UBox2DCollisionProfile;

class FBox2DCollisionProfileAssetTypeActions : public FAssetTypeActions_Base
{
public:
    FBox2DCollisionProfileAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

    // IAssetTypeActions interface
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
    virtual uint32 GetCategories() override;
    virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
    virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;
    // End of IAssetTypeActions interface

private:
    EAssetTypeCategories::Type MyAssetCategory;

    void ExecuteAddBody(TArray<TWeakObjectPtr<UBox2DCollisionProfile>> Objects);
    void ExecuteClearAll(TArray<TWeakObjectPtr<UBox2DCollisionProfile>> Objects);
};
