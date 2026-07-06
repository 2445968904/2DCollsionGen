#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/SBox2DCollisionEditorViewport.h"
#include "CollisionEditor/SBox2DCollisionEditorViewportToolbar.h"
#include "CollisionEditor/Box2DCollisionEditor.h"
#include "CollisionEditor/Box2DCollisionEditorCommands.h"
#include "CollisionEditor/Box2DCollisionEditorViewportClient.h"

void SBox2DCollisionEditorViewport::Construct(const FArguments& InArgs, TSharedPtr<FBox2DCollisionEditor> InEditor)
{
    EditorPtr = InEditor;
    SEditorViewport::Construct(SEditorViewport::FArguments());
}

void SBox2DCollisionEditorViewport::BindCommands()
{
    SEditorViewport::BindCommands();

    const FBox2DCollisionEditorCommands& Commands = FBox2DCollisionEditorCommands::Get();
    TSharedRef<FBox2DCollisionEditorViewportClient> ViewportClientRef = EditorViewportClient.ToSharedRef();

    // Show toggles
    CommandList->MapAction(
        Commands.SetShowGrid,
        FExecuteAction::CreateSP(ViewportClientRef, &FEditorViewportClient::SetShowGrid),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FEditorViewportClient::IsSetShowGridChecked));

    CommandList->MapAction(
        Commands.SetShowBounds,
        FExecuteAction::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::ToggleShowBounds),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::IsShowBoundsChecked));

    CommandList->MapAction(
        Commands.SetShowCollision,
        FExecuteAction::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::ToggleShowCollision),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::IsShowCollisionChecked));

    CommandList->MapAction(
        Commands.SetShowJoints,
        FExecuteAction::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::ToggleShowJoints),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::IsShowJointsChecked));

    CommandList->MapAction(
        Commands.SetShowSourceMesh,
        FExecuteAction::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::ToggleShowSourceMesh),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::IsShowSourceMeshChecked));

    // Mode switching - routed through the editor so FEdMode is activated/deactivated
    CommandList->MapAction(
        Commands.EnterViewMode,
        FExecuteAction::CreateLambda([this]() { if (EditorPtr.IsValid()) EditorPtr.Pin()->SetCurrentMode(EBox2DCollisionEditorMode::ViewMode); }),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::IsInViewMode));

    CommandList->MapAction(
        Commands.EnterEditShapesMode,
        FExecuteAction::CreateLambda([this]() { if (EditorPtr.IsValid()) EditorPtr.Pin()->SetCurrentMode(EBox2DCollisionEditorMode::EditShapesMode); }),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::IsInEditShapesMode));

    CommandList->MapAction(
        Commands.EnterEditJointsMode,
        FExecuteAction::CreateLambda([this]() { if (EditorPtr.IsValid()) EditorPtr.Pin()->SetCurrentMode(EBox2DCollisionEditorMode::EditJointsMode); }),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(ViewportClientRef, &FBox2DCollisionEditorViewportClient::IsInEditJointsMode));
}

TSharedRef<FEditorViewportClient> SBox2DCollisionEditorViewport::MakeEditorViewportClient()
{
    EditorViewportClient = MakeShareable(new FBox2DCollisionEditorViewportClient(EditorPtr, SharedThis(this)));
    return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SBox2DCollisionEditorViewport::BuildViewportToolbar()
{
    return SNew(SBox2DCollisionEditorViewportToolbar, SharedThis(this));
}

EVisibility SBox2DCollisionEditorViewport::GetTransformToolbarVisibility() const
{
    return EVisibility::Visible;
}

void SBox2DCollisionEditorViewport::OnFocusViewportToSelection()
{
    if (EditorViewportClient.IsValid())
    {
        EditorViewportClient->RequestFocusOnSelection(false);
    }
}

TSharedRef<SEditorViewport> SBox2DCollisionEditorViewport::GetViewportWidget()
{
    return SharedThis(this);
}

TSharedPtr<FExtender> SBox2DCollisionEditorViewport::GetExtenders() const
{
    return MakeShareable(new FExtender);
}

void SBox2DCollisionEditorViewport::OnFloatingButtonClicked()
{
}
