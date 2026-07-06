#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionEditor.h"
#include "CollisionEditor/Box2DCollisionEditorCommands.h"
#include "CollisionEditor/SBox2DCollisionEditorViewport.h"
#include "CollisionEditor/Box2DCollisionEditorViewportClient.h"
#include "CollisionEditor/SBox2DCollisionEditorViewportToolbar.h"
#include "CollisionEditor/Box2DBodyList.h"
#include "CollisionEditor/Box2DCollisionGeometryEditMode.h"
#include "CollisionEditor/Box2DCollisionGeometryEditCommands.h"
#include "CollisionEditor/Box2DCollisionJointEditMode.h"
#include "CollisionEditor/Box2DCollisionJointEditCommands.h"
#include "Box2DCollisionProfile.h"
#include "Box2DStyle.h"
#include "EditorModeManager.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Text/STextBlock.h"
#include "IDetailsView.h"
#include "SSingleObjectDetailsPanel.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionEditor"

static const FName Box2DCollisionEditorAppName(TEXT("Box2DCollisionEditorApp"));

struct FBox2DCollisionEditorTabs
{
    static const FName DetailsID;
    static const FName ViewportID;
    static const FName BodyListID;
};

const FName FBox2DCollisionEditorTabs::DetailsID(TEXT("Details"));
const FName FBox2DCollisionEditorTabs::ViewportID(TEXT("Viewport"));
const FName FBox2DCollisionEditorTabs::BodyListID(TEXT("BodyList"));

// Details panel
class SBox2DPropertiesTabBody : public SSingleObjectDetailsPanel
{
public:
    SLATE_BEGIN_ARGS(SBox2DPropertiesTabBody) {}
    SLATE_END_ARGS()

private:
    TWeakPtr<class FBox2DCollisionEditor> EditorPtr;

public:
    void Construct(const FArguments& InArgs, TSharedPtr<FBox2DCollisionEditor> InEditor)
    {
        EditorPtr = InEditor;
        SSingleObjectDetailsPanel::Construct(
            SSingleObjectDetailsPanel::FArguments()
            .HostCommandList(InEditor->GetToolkitCommands())
            .HostTabManager(InEditor->GetTabManager()),
            true, true);
    }

    virtual UObject* GetObjectToObserve() const override
    {
        return EditorPtr.Pin()->GetProfileBeingEdited();
    }

    virtual TSharedRef<SWidget> PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget) override
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .FillHeight(1)
            [
                PropertyEditorWidget
            ];
    }
};

//////////////////////////////////////////////////////////////////////////
// FBox2DCollisionEditor

FBox2DCollisionEditor::FBox2DCollisionEditor()
    : ProfileBeingEdited(nullptr)
{
}

TSharedRef<SDockTab> FBox2DCollisionEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .Label(LOCTEXT("ViewportTab_Title", "Viewport"))
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            [
                ViewportPtr.ToSharedRef()
            ]
            + SOverlay::Slot()
            .Padding(10)
            .VAlign(VAlign_Bottom)
            .HAlign(HAlign_Right)
            [
                SNew(STextBlock)
                .Visibility(EVisibility::HitTestInvisible)
                .TextStyle(FAppStyle::Get(), "Graph.CornerText")
                .Text(this, &FBox2DCollisionEditor::GetCurrentModeCornerText)
            ]
        ];
}

TSharedRef<SDockTab> FBox2DCollisionEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
    TSharedPtr<FBox2DCollisionEditor> EditorPtr = SharedThis(this);
    return SNew(SDockTab)
        .Label(LOCTEXT("DetailsTab_Title", "Details"))
        [
            SNew(SBox2DPropertiesTabBody, EditorPtr)
        ];
}

TSharedRef<SDockTab> FBox2DCollisionEditor::SpawnTab_BodyList(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .Label(LOCTEXT("BodyListTab_Title", "Bodies && Joints"))
        [
            BodyListPtr.ToSharedRef()
        ];
}

void FBox2DCollisionEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_Box2DCollisionEditor", "Box2D Collision Editor"));
    auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(FBox2DCollisionEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FBox2DCollisionEditor::SpawnTab_Viewport))
        .SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
        .SetGroup(WorkspaceMenuCategoryRef)
        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

    InTabManager->RegisterTabSpawner(FBox2DCollisionEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FBox2DCollisionEditor::SpawnTab_Details))
        .SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
        .SetGroup(WorkspaceMenuCategoryRef)
        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

    InTabManager->RegisterTabSpawner(FBox2DCollisionEditorTabs::BodyListID, FOnSpawnTab::CreateSP(this, &FBox2DCollisionEditor::SpawnTab_BodyList))
        .SetDisplayName(LOCTEXT("BodyListTabLabel", "Bodies && Joints"))
        .SetGroup(WorkspaceMenuCategoryRef)
        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.ContentBrowser"));
}

void FBox2DCollisionEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
    InTabManager->UnregisterTabSpawner(FBox2DCollisionEditorTabs::ViewportID);
    InTabManager->UnregisterTabSpawner(FBox2DCollisionEditorTabs::DetailsID);
    InTabManager->UnregisterTabSpawner(FBox2DCollisionEditorTabs::BodyListID);
}

void FBox2DCollisionEditor::InitCollisionEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UBox2DCollisionProfile* InitProfile)
{
    ProfileBeingEdited = InitProfile;

    FBox2DCollisionEditorCommands::Register();
    FBox2DCollisionGeometryEditCommands::Register();
    FBox2DCollisionJointEditCommands::Register();

    BindCommands();

    TSharedPtr<FBox2DCollisionEditor> EditorPtr = SharedThis(this);
    ViewportPtr = SNew(SBox2DCollisionEditorViewport, EditorPtr);
    BodyListPtr = SNew(SBox2DBodyList, EditorPtr);

    const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_Box2DCollisionEditor_Layout_v1")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Vertical)
            ->Split
            (
                FTabManager::NewSplitter()
                ->SetOrientation(Orient_Horizontal)
                ->SetSizeCoefficient(0.9f)
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.8f)
                    ->SetHideTabWell(true)
                    ->AddTab(FBox2DCollisionEditorTabs::ViewportID, ETabState::OpenedTab)
                )
                ->Split
                (
                    FTabManager::NewSplitter()
                    ->SetOrientation(Orient_Vertical)
                    ->SetSizeCoefficient(0.2f)
                    ->Split
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.6f)
                        ->SetHideTabWell(true)
                        ->AddTab(FBox2DCollisionEditorTabs::DetailsID, ETabState::OpenedTab)
                    )
                    ->Split
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.4f)
                        ->AddTab(FBox2DCollisionEditorTabs::BodyListID, ETabState::OpenedTab)
                    )
                )
            )
        );

    InitAssetEditor(Mode, InitToolkitHost, Box2DCollisionEditorAppName, StandaloneDefaultLayout, true, true, InitProfile);

    ExtendMenu();
    ExtendToolbar();
    RegenerateMenusAndToolbars();
}

void FBox2DCollisionEditor::BindCommands()
{
}

FName FBox2DCollisionEditor::GetToolkitFName() const
{
    return FName("Box2DCollisionEditor");
}

FText FBox2DCollisionEditor::GetBaseToolkitName() const
{
    return LOCTEXT("AppLabel", "Box2D Collision Editor");
}

FText FBox2DCollisionEditor::GetToolkitName() const
{
    return FText::FromString(ProfileBeingEdited->GetName());
}

FText FBox2DCollisionEditor::GetToolkitToolTipText() const
{
    return FAssetEditorToolkit::GetToolTipTextForObject(ProfileBeingEdited);
}

FString FBox2DCollisionEditor::GetWorldCentricTabPrefix() const
{
    return TEXT("Box2DCollisionEditor");
}

FString FBox2DCollisionEditor::GetDocumentationLink() const
{
    return TEXT("Physics/Box2D");
}

FLinearColor FBox2DCollisionEditor::GetWorldCentricTabColorScale() const
{
    return FLinearColor::White;
}

void FBox2DCollisionEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(ProfileBeingEdited);
}

void FBox2DCollisionEditor::ExtendMenu()
{
}

void FBox2DCollisionEditor::ExtendToolbar()
{
    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        ViewportPtr->GetCommandList(),
        FToolBarExtensionDelegate::CreateSP(this, &FBox2DCollisionEditor::CreateModeToolbarWidgets));

    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        ViewportPtr->GetCommandList(),
        FToolBarExtensionDelegate::CreateSP(this, &FBox2DCollisionEditor::CreateShapeToolbarWidgets));

    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        ViewportPtr->GetCommandList(),
        FToolBarExtensionDelegate::CreateSP(this, &FBox2DCollisionEditor::CreateJointToolbarWidgets));

    AddToolbarExtender(ToolbarExtender);
}

void FBox2DCollisionEditor::CreateEditorModeManager()
{
    check(ViewportPtr.IsValid());
    TSharedPtr<FEditorViewportClient> ViewportClient = ViewportPtr->GetViewportClient();
    check(ViewportClient.IsValid());
    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    ViewportClient->TakeOwnershipOfModeManager(EditorModeManager);
    PRAGMA_ENABLE_DEPRECATION_WARNINGS

    // Register and activate the geometry edit mode
    EditorModeManager->SetDefaultMode(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry);
    EditorModeManager->ActivateDefaultMode();

    // Configure the mode while it's still active
    if (FBox2DCollisionGeometryEditMode* GeometryMode = EditorModeManager->GetActiveModeTyped<FBox2DCollisionGeometryEditMode>(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry))
    {
        GeometryMode->SetProfileBeingEdited(ProfileBeingEdited);
        GeometryMode->BindCommands(ViewportPtr->GetCommandList());
    }

    // Deactivate the mode initially (View mode by default)
    EditorModeManager->DeactivateMode(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry);

    // Activate and configure joint edit mode (for command binding), then deactivate
    EditorModeManager->ActivateMode(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint);
    if (FBox2DCollisionJointEditMode* JointMode = EditorModeManager->GetActiveModeTyped<FBox2DCollisionJointEditMode>(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint))
    {
        JointMode->SetProfileBeingEdited(ProfileBeingEdited);
        JointMode->BindCommands(ViewportPtr->GetCommandList());
    }
    EditorModeManager->DeactivateMode(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint);
}

void FBox2DCollisionEditor::CreateModeToolbarWidgets(FToolBarBuilder& IgnoredBuilder)
{
    FSlimHorizontalToolBarBuilder ToolbarBuilder(ViewportPtr->GetCommandList(), FMultiBoxCustomization::None);
    const FBox2DCollisionEditorCommands& Commands = FBox2DCollisionEditorCommands::Get();
    ToolbarBuilder.AddToolBarButton(Commands.EnterViewMode);
    ToolbarBuilder.AddToolBarButton(Commands.EnterEditShapesMode);
    ToolbarBuilder.AddToolBarButton(Commands.EnterEditJointsMode);
    AddToolbarWidget(ToolbarBuilder.MakeWidget());
}

void FBox2DCollisionEditor::CreateShapeToolbarWidgets(FToolBarBuilder& IgnoredBuilder)
{
    FSlimHorizontalToolBarBuilder ToolbarBuilder(ViewportPtr->GetCommandList(), FMultiBoxCustomization::None);
    const FBox2DCollisionGeometryEditCommands& Commands = FBox2DCollisionGeometryEditCommands::Get();
    ToolbarBuilder.AddToolBarButton(Commands.AddBoxShape);
    ToolbarBuilder.AddToolBarButton(Commands.AddCircleShape);
    ToolbarBuilder.AddToolBarButton(Commands.AddPolygonShape);
    ToolbarBuilder.AddToolBarButton(Commands.DeleteSelection);
    AddToolbarWidget(ToolbarBuilder.MakeWidget());
}

void FBox2DCollisionEditor::CreateJointToolbarWidgets(FToolBarBuilder& IgnoredBuilder)
{
    FSlimHorizontalToolBarBuilder ToolbarBuilder(ViewportPtr->GetCommandList(), FMultiBoxCustomization::None);
    const FBox2DCollisionJointEditCommands& Commands = FBox2DCollisionJointEditCommands::Get();
    ToolbarBuilder.AddToolBarButton(Commands.AddDistanceJoint);
    ToolbarBuilder.AddToolBarButton(Commands.AddRevoluteJoint);
    ToolbarBuilder.AddToolBarButton(Commands.AddPrismaticJoint);
    ToolbarBuilder.AddToolBarButton(Commands.AddWeldJoint);
    ToolbarBuilder.AddToolBarButton(Commands.DeleteJoint);
    AddToolbarWidget(ToolbarBuilder.MakeWidget());
}

void FBox2DCollisionEditor::SetCurrentMode(EBox2DCollisionEditorMode::Type NewMode)
{
    if (!ViewportPtr.IsValid()) return;

    TSharedPtr<FEditorViewportClient> VC = ViewportPtr->GetViewportClient();
    if (FBox2DCollisionEditorViewportClient* TypedVC = StaticCastSharedPtr<FBox2DCollisionEditorViewportClient>(VC).Get())
    {
        switch (NewMode)
        {
        case EBox2DCollisionEditorMode::ViewMode:
            TypedVC->EnterViewMode();
            EditorModeManager->DeactivateMode(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry);
            EditorModeManager->DeactivateMode(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint);
            break;
        case EBox2DCollisionEditorMode::EditShapesMode:
            TypedVC->EnterEditShapesMode();
            EditorModeManager->DeactivateMode(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint);
            if (!EditorModeManager->IsModeActive(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry))
            {
                EditorModeManager->ActivateMode(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry);
            }
            if (FBox2DCollisionGeometryEditMode* GeometryMode = EditorModeManager->GetActiveModeTyped<FBox2DCollisionGeometryEditMode>(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry))
            {
                GeometryMode->SetProfileBeingEdited(ProfileBeingEdited);
            }
            TypedVC->SetWidgetMode(UE::Widget::WM_Translate);
            break;
        case EBox2DCollisionEditorMode::EditJointsMode:
            TypedVC->EnterEditJointsMode();
            EditorModeManager->DeactivateMode(FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry);
            if (!EditorModeManager->IsModeActive(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint))
            {
                EditorModeManager->ActivateMode(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint);
            }
            if (FBox2DCollisionJointEditMode* JointMode = EditorModeManager->GetActiveModeTyped<FBox2DCollisionJointEditMode>(FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint))
            {
                JointMode->SetProfileBeingEdited(ProfileBeingEdited);
            }
            TypedVC->SetWidgetMode(UE::Widget::WM_Translate);
            break;
        }
    }
}

EBox2DCollisionEditorMode::Type FBox2DCollisionEditor::GetCurrentMode() const
{
    if (ViewportPtr.IsValid())
    {
        TSharedPtr<FEditorViewportClient> VC = ViewportPtr->GetViewportClient();
        if (FBox2DCollisionEditorViewportClient* TypedVC = StaticCastSharedPtr<FBox2DCollisionEditorViewportClient>(VC).Get())
        {
            return TypedVC->GetCurrentMode();
        }
    }
    return EBox2DCollisionEditorMode::ViewMode;
}

FText FBox2DCollisionEditor::GetCurrentModeCornerText() const
{
    switch (GetCurrentMode())
    {
    case EBox2DCollisionEditorMode::EditShapesMode:
        return LOCTEXT("EditShapes_CornerText", "Edit Shapes");
    case EBox2DCollisionEditorMode::EditJointsMode:
        return LOCTEXT("EditJoints_CornerText", "Edit Joints");
    default:
        return FText::GetEmpty();
    }
}

#undef LOCTEXT_NAMESPACE
