#pragma once

#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"

class FSpawnTabArgs;
class SBox2DCollisionEditorViewport;
class SBox2DBodyList;
class UBox2DCollisionProfile;

namespace EBox2DCollisionEditorMode
{
    enum Type
    {
        ViewMode,
        EditShapesMode,
        EditJointsMode
    };
}

class FBox2DCollisionEditor : public FAssetEditorToolkit, public FGCObject
{
public:
    FBox2DCollisionEditor();

    // IToolkit interface
    virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

    // FAssetEditorToolkit
    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FText GetToolkitName() const override;
    virtual FText GetToolkitToolTipText() const override;
    virtual FLinearColor GetWorldCentricTabColorScale() const override;
    virtual FString GetWorldCentricTabPrefix() const override;
    virtual FString GetDocumentationLink() const override;
    // End of FAssetEditorToolkit

    // FGCObject interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual FString GetReferencerName() const override { return TEXT("FBox2DCollisionEditor"); }
    // End of FGCObject interface

    void InitCollisionEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UBox2DCollisionProfile* InitProfile);

    UBox2DCollisionProfile* GetProfileBeingEdited() const { return ProfileBeingEdited; }
    EBox2DCollisionEditorMode::Type GetCurrentMode() const;

public:
    void SetCurrentMode(EBox2DCollisionEditorMode::Type NewMode);

protected:
    TObjectPtr<UBox2DCollisionProfile> ProfileBeingEdited;
    TSharedPtr<SBox2DCollisionEditorViewport> ViewportPtr;
    TSharedPtr<SBox2DBodyList> BodyListPtr;

protected:
    void BindCommands();
    void ExtendMenu();
    void ExtendToolbar();
    virtual void CreateEditorModeManager() override;

    TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_BodyList(const FSpawnTabArgs& Args);

    void CreateModeToolbarWidgets(FToolBarBuilder& ToolbarBuilder);
    void CreateShapeToolbarWidgets(FToolBarBuilder& ToolbarBuilder);
    void CreateJointToolbarWidgets(FToolBarBuilder& ToolbarBuilder);
    FText GetCurrentModeCornerText() const;
};
