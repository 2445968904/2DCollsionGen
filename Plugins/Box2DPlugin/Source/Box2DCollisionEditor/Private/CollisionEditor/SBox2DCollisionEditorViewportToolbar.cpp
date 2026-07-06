#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/SBox2DCollisionEditorViewportToolbar.h"
#include "CollisionEditor/Box2DCollisionEditorCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "SEditorViewport.h"

void SBox2DCollisionEditorViewportToolbar::Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
{
    SCommonEditorViewportToolbarBase::Construct(SCommonEditorViewportToolbarBase::FArguments(), InInfoProvider);
}

TSharedRef<SWidget> SBox2DCollisionEditorViewportToolbar::GenerateShowMenu() const
{
    GetInfoProvider().OnFloatingButtonClicked();

    TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

    const bool bInShouldCloseWindowAfterMenuSelection = true;
    FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
    {
        const FBox2DCollisionEditorCommands& Commands = FBox2DCollisionEditorCommands::Get();

        ShowMenuBuilder.AddMenuEntry(Commands.SetShowGrid);
        ShowMenuBuilder.AddMenuEntry(Commands.SetShowBounds);

        ShowMenuBuilder.AddMenuSeparator();

        ShowMenuBuilder.AddMenuEntry(Commands.SetShowCollision);
        ShowMenuBuilder.AddMenuEntry(Commands.SetShowJoints);
    }

    return ShowMenuBuilder.MakeWidget();
}
