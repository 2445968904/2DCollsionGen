#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionJointEditMode.h"
#include "CollisionEditor/Box2DCollisionJointEditCommands.h"
#include "CollisionEditor/Box2DCollisionJointSelection.h"
#include "CollisionEditor/Box2DEditorUtils.h"
#include "Box2DCollisionProfile.h"
#include "Box2DCollisionTypes.h"
#include "EditorViewportClient.h"
#include "Framework/Commands/UICommandList.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "EditorModeManager.h"
#include "SceneView.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionJointEditMode"

const FEditorModeID FBox2DCollisionJointEditMode::EM_Box2DCollisionJoint(TEXT("Box2DCollisionJointEditMode"));

namespace Box2DJointEditColors
{
    const FLinearColor SelectionColor(1.0f, 0.8f, 0.0f, 1.0f);
    const FLinearColor AnchorHighlight(1.0f, 0.5f, 0.0f, 1.0f);
    const float VertexSize = 8.0f;
}

using namespace Box2DEditorUtils;

FBox2DCollisionJointEditMode::FBox2DCollisionJointEditMode()
    : ProfileBeingEdited(nullptr)
    , bIsCreatingJoint(false)
    , PendingJointType(EBox2DCollisionJointType::Revolute)
    , PendingBodyAIndex(INDEX_NONE)
{
    bDrawPivot = false;
    bDrawGrid = false;
}

void FBox2DCollisionJointEditMode::Initialize()
{
}

void FBox2DCollisionJointEditMode::SetProfileBeingEdited(UBox2DCollisionProfile* InProfile)
{
    ProfileBeingEdited = InProfile;
    ClearSelection();
    AbandonCreateJoint();
}

void FBox2DCollisionJointEditMode::BindCommands(TSharedPtr<FUICommandList> InCommandList)
{
    const FBox2DCollisionJointEditCommands& Commands = FBox2DCollisionJointEditCommands::Get();

    InCommandList->MapAction(
        Commands.AddDistanceJoint,
        FExecuteAction::CreateSP(this, &FBox2DCollisionJointEditMode::StartCreateJoint, EBox2DCollisionJointType::Distance));

    InCommandList->MapAction(
        Commands.AddRevoluteJoint,
        FExecuteAction::CreateSP(this, &FBox2DCollisionJointEditMode::StartCreateJoint, EBox2DCollisionJointType::Revolute));

    InCommandList->MapAction(
        Commands.AddPrismaticJoint,
        FExecuteAction::CreateSP(this, &FBox2DCollisionJointEditMode::StartCreateJoint, EBox2DCollisionJointType::Prismatic));

    InCommandList->MapAction(
        Commands.AddWeldJoint,
        FExecuteAction::CreateSP(this, &FBox2DCollisionJointEditMode::StartCreateJoint, EBox2DCollisionJointType::Weld));

    InCommandList->MapAction(
        Commands.DeleteJoint,
        FExecuteAction::CreateSP(this, &FBox2DCollisionJointEditMode::DeleteSelectedItems),
        FCanExecuteAction::CreateSP(this, &FBox2DCollisionJointEditMode::HasSelection));
}

FVector2D FBox2DCollisionJointEditMode::ViewportClickTo2D(const FViewportClick& Click, FEditorViewportClient* ViewportClient) const
{
    const FPlane ClickPlane(FVector(0, 1, 0), 0);
    const FVector WorldPoint = FMath::LinePlaneIntersection(Click.GetOrigin(), Click.GetOrigin() + Click.GetDirection(), ClickPlane);
    return WorldTo2D(WorldPoint);
}

FVector2D FBox2DCollisionJointEditMode::WorldTo2D(const FVector& WorldPos) const
{
    if (ProfileBeingEdited)
    {
        return FromXZPlane(WorldPos) / ProfileBeingEdited->PixelsPerMeter;
    }
    return FromXZPlane(WorldPos) / 100.0f;
}

FVector FBox2DCollisionJointEditMode::TwoDToWorld(const FVector2D& Pos2D) const
{
    if (ProfileBeingEdited)
    {
        return ToXZPlane(Pos2D * ProfileBeingEdited->PixelsPerMeter);
    }
    return ToXZPlane(Pos2D * 100.0f);
}

void FBox2DCollisionJointEditMode::ClearSelection()
{
    SelectedItems.Empty();
}

void FBox2DCollisionJointEditMode::AddJoint(EBox2DCollisionJointType JointType)
{
    if (!ProfileBeingEdited || ProfileBeingEdited->Bodies.Num() < 2) return;

    const FScopedTransaction Transaction(LOCTEXT("AddJoint", "Add Joint"));
    ProfileBeingEdited->Modify();

    // Default: connect first two bodies
    FBox2DCollisionJoint NewJoint;
    NewJoint.JointType = JointType;
    NewJoint.BodyAName = ProfileBeingEdited->Bodies[0].BodyName;
    NewJoint.BodyBName = ProfileBeingEdited->Bodies[1].BodyName;
    NewJoint.JointName = *FString::Printf(TEXT("Joint_%d"), ProfileBeingEdited->Joints.Num());

    // Set default anchors at body positions
    NewJoint.AnchorA = ProfileBeingEdited->Bodies[0].Position;
    NewJoint.AnchorB = ProfileBeingEdited->Bodies[1].Position;

    ProfileBeingEdited->Joints.Add(NewJoint);

    ClearSelection();
    SelectedItems.Add(MakeShared<FBox2DSelectedJoint>(ProfileBeingEdited->Joints.Num() - 1));
}

void FBox2DCollisionJointEditMode::StartCreateJoint(EBox2DCollisionJointType JointType)
{
    bIsCreatingJoint = true;
    PendingJointType = JointType;
    PendingBodyAIndex = INDEX_NONE;
}

void FBox2DCollisionJointEditMode::AbandonCreateJoint()
{
    bIsCreatingJoint = false;
    PendingBodyAIndex = INDEX_NONE;
}

void FBox2DCollisionJointEditMode::DeleteSelectedItems()
{
    if (!ProfileBeingEdited || SelectedItems.Num() == 0) return;

    const FScopedTransaction Transaction(LOCTEXT("DeleteJoint", "Delete Joint"));
    ProfileBeingEdited->Modify();

    TSet<int32> JointsToDelete;
    for (const TSharedPtr<FBox2DSelectedJointItem>& Item : SelectedItems)
    {
        JointsToDelete.Add(Item->JointIndex);
    }

    TArray<int32> Sorted = JointsToDelete.Array();
    Sorted.Sort([](int32 A, int32 B) { return A > B; });
    for (int32 Idx : Sorted)
    {
        if (ProfileBeingEdited->Joints.IsValidIndex(Idx))
        {
            ProfileBeingEdited->Joints.RemoveAt(Idx);
        }
    }

    ClearSelection();
}

bool FBox2DCollisionJointEditMode::ShouldDrawWidget() const
{
    return HasSelection();
}

FVector FBox2DCollisionJointEditMode::GetWidgetLocation() const
{
    if (SelectedItems.Num() == 0 || !ProfileBeingEdited) return FVector::ZeroVector;

    FVector Sum(ForceInitToZero);
    for (const TSharedPtr<FBox2DSelectedJointItem>& Item : SelectedItems)
    {
        Sum += Item->GetWorldPos(ProfileBeingEdited);
    }
    return Sum / SelectedItems.Num();
}

bool FBox2DCollisionJointEditMode::UsesTransformWidget(UE::Widget::EWidgetMode CheckMode) const
{
    return CheckMode == UE::Widget::WM_Translate || CheckMode == UE::Widget::WM_None;
}

bool FBox2DCollisionJointEditMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
    const bool bManipulating = GetModeManager()->IsTracking();
    EAxisList::Type CurrentAxis = GetCurrentWidgetAxis();

    if (bManipulating && CurrentAxis != EAxisList::None && SelectedItems.Num() > 0 && ProfileBeingEdited)
    {
        FVector2D Drag2D(InDrag.X, InDrag.Z);

        ProfileBeingEdited->Modify();

        for (TSharedPtr<FBox2DSelectedJointItem>& Item : SelectedItems)
        {
            Item->ApplyDelta(ProfileBeingEdited, Drag2D);
        }
        return true;
    }

    return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
}

bool FBox2DCollisionJointEditMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
    if (!ProfileBeingEdited) return false;

    FViewport* Viewport = InViewportClient->Viewport;
    const bool bIsCtrlKeyDown = Viewport->KeyState(EKeys::LeftControl) || Viewport->KeyState(EKeys::RightControl);
    const bool bIsShiftKeyDown = Viewport->KeyState(EKeys::LeftShift) || Viewport->KeyState(EKeys::RightShift);

    if (Click.GetEvent() != IE_Pressed && Click.GetEvent() != IE_Released) return false;

    // Joint creation workflow: click body A then body B
    if (bIsCreatingJoint)
    {
        // Find which body was clicked
        float Scale = ProfileBeingEdited->PixelsPerMeter;
        FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
            Viewport, InViewportClient->GetScene(), InViewportClient->EngineShowFlags));
        FSceneView* View = InViewportClient->CalcSceneView(&ViewFamily);
        FVector2D ClickScreen(Click.GetCursorPos().X, Click.GetCursorPos().Y);
        float BestDist = 25.0f;
        int32 BestBody = INDEX_NONE;

        for (int32 i = 0; i < ProfileBeingEdited->Bodies.Num(); i++)
        {
            FVector2D BodyScreen;
            View->WorldToPixel(ToXZPlane(ProfileBeingEdited->Bodies[i].Position * Scale), BodyScreen);
            float Dist = (BodyScreen - ClickScreen).Size();
            if (Dist < BestDist)
            {
                BestDist = Dist;
                BestBody = i;
            }
        }

        if (BestBody != INDEX_NONE)
        {
            if (PendingBodyAIndex == INDEX_NONE)
            {
                PendingBodyAIndex = BestBody;
            }
            else if (BestBody != PendingBodyAIndex)
            {
                // Create the joint between BodyA and BodyB
                const FScopedTransaction Transaction(LOCTEXT("CreateJoint", "Create Joint"));
                ProfileBeingEdited->Modify();

                FBox2DCollisionJoint NewJoint;
                NewJoint.JointType = PendingJointType;
                NewJoint.BodyAName = ProfileBeingEdited->Bodies[PendingBodyAIndex].BodyName;
                NewJoint.BodyBName = ProfileBeingEdited->Bodies[BestBody].BodyName;
                NewJoint.JointName = *FString::Printf(TEXT("Joint_%d"), ProfileBeingEdited->Joints.Num());
                NewJoint.AnchorA = ProfileBeingEdited->Bodies[PendingBodyAIndex].Position;
                NewJoint.AnchorB = ProfileBeingEdited->Bodies[BestBody].Position;

                ProfileBeingEdited->Joints.Add(NewJoint);

                ClearSelection();
                SelectedItems.Add(MakeShared<FBox2DSelectedAnchorA>(ProfileBeingEdited->Joints.Num() - 1));

                bIsCreatingJoint = false;
                PendingBodyAIndex = INDEX_NONE;
            }
        }
        return true;
    }

    // Normal selection: click on joints and anchors
    float Scale = ProfileBeingEdited->PixelsPerMeter;
    FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
        Viewport, InViewportClient->GetScene(), InViewportClient->EngineShowFlags));
    FSceneView* View = InViewportClient->CalcSceneView(&ViewFamily);
    FVector2D ClickScreen(Click.GetCursorPos().X, Click.GetCursorPos().Y);

    float BestDist = 15.0f;
    TSharedPtr<FBox2DSelectedJointItem> BestHit;

    for (int32 JointIdx = 0; JointIdx < ProfileBeingEdited->Joints.Num(); JointIdx++)
    {
        const FBox2DCollisionJoint& Joint = ProfileBeingEdited->Joints[JointIdx];

        int32 BodyAIdx = ProfileBeingEdited->FindBodyIndex(Joint.BodyAName);
        int32 BodyBIdx = ProfileBeingEdited->FindBodyIndex(Joint.BodyBName);

        // Check Anchor A
        if (BodyAIdx != INDEX_NONE)
        {
            FVector2D AnchorAW = (ProfileBeingEdited->Bodies[BodyAIdx].Position + Joint.AnchorA) * Scale;
            FVector2D ScreenPos;
            View->WorldToPixel(ToXZPlane(AnchorAW), ScreenPos);
            float Dist = (ScreenPos - ClickScreen).Size();
            if (Dist < BestDist)
            {
                BestDist = Dist;
                BestHit = MakeShared<FBox2DSelectedAnchorA>(JointIdx);
            }
        }

        // Check Anchor B
        if (BodyBIdx != INDEX_NONE)
        {
            FVector2D AnchorBW = (ProfileBeingEdited->Bodies[BodyBIdx].Position + Joint.AnchorB) * Scale;
            FVector2D ScreenPos;
            View->WorldToPixel(ToXZPlane(AnchorBW), ScreenPos);
            float Dist = (ScreenPos - ClickScreen).Size();
            if (Dist < BestDist)
            {
                BestDist = Dist;
                BestHit = MakeShared<FBox2DSelectedAnchorB>(JointIdx);
            }
        }

        // Check midpoint for whole-joint selection
        if (BodyAIdx != INDEX_NONE && BodyBIdx != INDEX_NONE)
        {
            FVector2D AnchorAW = (ProfileBeingEdited->Bodies[BodyAIdx].Position + Joint.AnchorA) * Scale;
            FVector2D AnchorBW = (ProfileBeingEdited->Bodies[BodyBIdx].Position + Joint.AnchorB) * Scale;
            FVector2D Mid = (AnchorAW + AnchorBW) * 0.5f;
            FVector2D ScreenPos;
            View->WorldToPixel(ToXZPlane(Mid), ScreenPos);
            float Dist = (ScreenPos - ClickScreen).Size();
            if (Dist < BestDist)
            {
                BestDist = Dist;
                BestHit = MakeShared<FBox2DSelectedJoint>(JointIdx);
            }
        }
    }

    if (BestHit.IsValid())
    {
        if (!bIsCtrlKeyDown && !bIsShiftKeyDown)
        {
            ClearSelection();
        }
        SelectedItems.Add(BestHit);
    }
    else
    {
        if (!bIsShiftKeyDown)
        {
            ClearSelection();
        }
    }

    return BestHit.IsValid() ? true : FEdMode::HandleClick(InViewportClient, HitProxy, Click);
}

bool FBox2DCollisionJointEditMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
    bool bHandled = false;

    if (bIsCreatingJoint)
    {
        if (Key == EKeys::Escape && Event == IE_Pressed)
        {
            AbandonCreateJoint();
            bHandled = true;
        }
    }

    if (Key == EKeys::Delete && Event == IE_Pressed && HasSelection())
    {
        DeleteSelectedItems();
        bHandled = true;
    }

    return bHandled ? true : FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

void FBox2DCollisionJointEditMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
    FEdMode::Render(View, Viewport, PDI);
    DrawJointSelection(PDI);
}

void FBox2DCollisionJointEditMode::DrawJointSelection(FPrimitiveDrawInterface* PDI)
{
    if (!ProfileBeingEdited) return;

    for (const TSharedPtr<FBox2DSelectedJointItem>& Item : SelectedItems)
    {
        if (const FBox2DSelectedAnchorA* AnchorA = Item->CastTo<FBox2DSelectedAnchorA>(FBox2DJointSelectionTypes::AnchorA))
        {
            FVector WorldPos = Item->GetWorldPos(ProfileBeingEdited);
            PDI->DrawPoint(WorldPos, Box2DJointEditColors::AnchorHighlight, Box2DJointEditColors::VertexSize, SDPG_Foreground);
        }
        else if (const FBox2DSelectedAnchorB* AnchorB = Item->CastTo<FBox2DSelectedAnchorB>(FBox2DJointSelectionTypes::AnchorB))
        {
            FVector WorldPos = Item->GetWorldPos(ProfileBeingEdited);
            PDI->DrawPoint(WorldPos, Box2DJointEditColors::AnchorHighlight, Box2DJointEditColors::VertexSize, SDPG_Foreground);
        }
        else if (const FBox2DSelectedJoint* JointSel = Item->CastTo<FBox2DSelectedJoint>(FBox2DJointSelectionTypes::Joint))
        {
            const FBox2DCollisionJoint& Joint = ProfileBeingEdited->Joints[JointSel->JointIndex];
            int32 BodyAIdx = ProfileBeingEdited->FindBodyIndex(Joint.BodyAName);
            int32 BodyBIdx = ProfileBeingEdited->FindBodyIndex(Joint.BodyBName);
            float Scale = ProfileBeingEdited->PixelsPerMeter;

            if (BodyAIdx != INDEX_NONE && BodyBIdx != INDEX_NONE)
            {
                FVector2D AnchorAW = (ProfileBeingEdited->Bodies[BodyAIdx].Position + Joint.AnchorA) * Scale;
                FVector2D AnchorBW = (ProfileBeingEdited->Bodies[BodyBIdx].Position + Joint.AnchorB) * Scale;
                PDI->DrawLine(ToXZPlane(AnchorAW), ToXZPlane(AnchorBW), Box2DJointEditColors::SelectionColor, SDPG_Foreground);
                PDI->DrawPoint(ToXZPlane(AnchorAW), Box2DJointEditColors::SelectionColor, Box2DJointEditColors::VertexSize, SDPG_Foreground);
                PDI->DrawPoint(ToXZPlane(AnchorBW), Box2DJointEditColors::SelectionColor, Box2DJointEditColors::VertexSize, SDPG_Foreground);
            }
        }
    }
}

void FBox2DCollisionJointEditMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
    FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);

    if (bIsCreatingJoint)
    {
        FString Msg;
        if (PendingBodyAIndex == INDEX_NONE)
        {
            Msg = TEXT("Joint Creation: Click on Body A");
        }
        else
        {
            Msg = FString::Printf(TEXT("Joint Creation: Body A = %s, now click on Body B (Esc to cancel)"),
                *ProfileBeingEdited->Bodies[PendingBodyAIndex].BodyName.ToString());
        }

        FCanvasTextItem TextItem(FVector2D(6, 24), FText::AsCultureInvariant(Msg),
            GEngine->GetSmallFont(), Box2DJointEditColors::AnchorHighlight);
        TextItem.EnableShadow(FLinearColor::Black);
        TextItem.Draw(Canvas);
    }
}

#undef LOCTEXT_NAMESPACE
