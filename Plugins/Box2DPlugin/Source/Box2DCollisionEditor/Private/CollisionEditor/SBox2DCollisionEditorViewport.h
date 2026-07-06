#pragma once

#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class FBox2DCollisionEditor;
class FBox2DCollisionEditorViewportClient;

class SBox2DCollisionEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
    SLATE_BEGIN_ARGS(SBox2DCollisionEditorViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<FBox2DCollisionEditor> InEditor);

    // SEditorViewport interface
    virtual void BindCommands() override;
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual TSharedPtr<SWidget> BuildViewportToolbar() override;
    virtual EVisibility GetTransformToolbarVisibility() const override;
    virtual void OnFocusViewportToSelection() override;
    // End of SEditorViewport interface

    // ICommonEditorViewportToolbarInfoProvider interface
    virtual TSharedRef<SEditorViewport> GetViewportWidget() override;
    virtual TSharedPtr<FExtender> GetExtenders() const override;
    virtual void OnFloatingButtonClicked() override;
    // End of ICommonEditorViewportToolbarInfoProvider interface

private:
    TWeakPtr<FBox2DCollisionEditor> EditorPtr;
    TSharedPtr<FBox2DCollisionEditorViewportClient> EditorViewportClient;
};
