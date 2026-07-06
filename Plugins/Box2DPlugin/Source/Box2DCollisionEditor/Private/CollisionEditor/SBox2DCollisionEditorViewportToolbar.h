#pragma once

#include "SCommonEditorViewportToolbarBase.h"

class SBox2DCollisionEditorViewportToolbar : public SCommonEditorViewportToolbarBase
{
public:
    SLATE_BEGIN_ARGS(SBox2DCollisionEditorViewportToolbar) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider);

    // SCommonEditorViewportToolbarBase interface
    virtual TSharedRef<SWidget> GenerateShowMenu() const override;
    // End of SCommonEditorViewportToolbarBase
};
