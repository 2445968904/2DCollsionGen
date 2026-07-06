#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionGeometryEditMode.h"
#include "CollisionEditor/Box2DCollisionGeometryEditCommands.h"
#include "CollisionEditor/Box2DCollisionSelection.h"
#include "Box2DCollisionProfile.h"
#include "Box2DCollisionTypes.h"
#include "EditorViewportClient.h"
#include "Framework/Commands/UICommandList.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "EditorModeManager.h"
#include "SceneView.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionGeometryEditMode"

const FEditorModeID FBox2DCollisionGeometryEditMode::EM_Box2DCollisionGeometry(TEXT("Box2DCollisionGeometryEditMode"));

namespace Box2DEditModeColors
{
    const FLinearColor SelectionColor(1.0f, 0.8f, 0.0f, 1.0f);
    const FLinearColor PolygonPreviewColor(0.0f, 1.0f, 0.5f, 0.8f);
    const float VertexSize = 6.0f;
}

// 2D↔3D helpers for XZ plane
static FORCEINLINE FVector ToXZPlane(const FVector2D& Pos2D)
{
    return FVector(Pos2D.X, 0.0f, Pos2D.Y);
}

static FORCEINLINE FVector2D FromXZPlane(const FVector& WorldPos)
{
    return FVector2D(WorldPos.X, WorldPos.Z);
}

FBox2DCollisionGeometryEditMode::FBox2DCollisionGeometryEditMode()
    : ProfileBeingEdited(nullptr)
    , bIsAddingPolygon(false)
    , ActiveBodyIndex(0)
{
    bDrawPivot = false;
    bDrawGrid = false;
}

void FBox2DCollisionGeometryEditMode::Initialize()
{
}

void FBox2DCollisionGeometryEditMode::SetProfileBeingEdited(UBox2DCollisionProfile* InProfile)
{
    ProfileBeingEdited = InProfile;
    ClearSelection();
    AbandonAddPolygon();
}

void FBox2DCollisionGeometryEditMode::BindCommands(TSharedPtr<FUICommandList> InCommandList)
{
    const FBox2DCollisionGeometryEditCommands& Commands = FBox2DCollisionGeometryEditCommands::Get();

    InCommandList->MapAction(
        Commands.AddBoxShape,
        FExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::AddBoxShape),
        FCanExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::CanAddShape));

    InCommandList->MapAction(
        Commands.AddCircleShape,
        FExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::AddCircleShape),
        FCanExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::CanAddShape));

    InCommandList->MapAction(
        Commands.AddPolygonShape,
        FExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::ToggleAddPolygonMode),
        FCanExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::CanAddShape),
        FIsActionChecked::CreateSP(this, &FBox2DCollisionGeometryEditMode::IsAddingPolygon));

    InCommandList->MapAction(
        Commands.DeleteSelection,
        FExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::DeleteSelectedItems),
        FCanExecuteAction::CreateSP(this, &FBox2DCollisionGeometryEditMode::HasSelection));
}

// Convert a viewport click to 2D position in meters
FVector2D FBox2DCollisionGeometryEditMode::ViewportClickTo2D(const FViewportClick& Click, FEditorViewportClient* ViewportClient) const
{
    // Get the world position from the click by intersecting with the Y=0 plane
    const FPlane ClickPlane(FVector(0, 1, 0), 0); // Y=0 plane
    const FVector WorldPoint = FMath::LinePlaneIntersection(Click.GetOrigin(), Click.GetOrigin() + Click.GetDirection(), ClickPlane);
    return WorldTo2D(WorldPoint);
}

FVector2D FBox2DCollisionGeometryEditMode::WorldTo2D(const FVector& WorldPos) const
{
    if (ProfileBeingEdited)
    {
        return FromXZPlane(WorldPos) / ProfileBeingEdited->PixelsPerMeter;
    }
    return FromXZPlane(WorldPos) / 100.0f;
}

FVector FBox2DCollisionGeometryEditMode::TwoDToWorld(const FVector2D& Pos2D) const
{
    if (ProfileBeingEdited)
    {
        return ToXZPlane(Pos2D * ProfileBeingEdited->PixelsPerMeter);
    }
    return ToXZPlane(Pos2D * 100.0f);
}

void FBox2DCollisionGeometryEditMode::ClearSelection()
{
    SelectedItems.Empty();
}

void FBox2DCollisionGeometryEditMode::AddToSelection(TSharedPtr<FBox2DSelectedItem> Item)
{
    SelectedItems.Add(Item);
}

void FBox2DCollisionGeometryEditMode::DeleteSelectedItems()
{
    if (!ProfileBeingEdited || SelectedItems.Num() == 0) return;

    const FScopedTransaction Transaction(LOCTEXT("DeleteSelection", "Delete Selection"));
    ProfileBeingEdited->Modify();

    // Collect unique shape/body indices to delete (in reverse order to maintain validity)
    TSet<int32> BodiesToDelete;
    TSet<TPair<int32, int32>> ShapesToDelete;

    for (const TSharedPtr<FBox2DSelectedItem>& Item : SelectedItems)
    {
        if (const FBox2DSelectedShape* ShapeSel = Item->CastTo<FBox2DSelectedShape>(FBox2DCollisionSelectionTypes::Shape))
        {
            ShapesToDelete.Add(TPair<int32, int32>(ShapeSel->BodyIndex, ShapeSel->ShapeIndex));
        }
        else if (const FBox2DSelectedBody* BodySel = Item->CastTo<FBox2DSelectedBody>(FBox2DCollisionSelectionTypes::Body))
        {
            BodiesToDelete.Add(BodySel->BodyIndex);
        }
        else if (const FBox2DSelectedVertex* VertSel = Item->CastTo<FBox2DSelectedVertex>(FBox2DCollisionSelectionTypes::Vertex))
        {
            // For polygon vertices, delete the vertex; for box/circle, delete the shape
            if (ProfileBeingEdited->Bodies.IsValidIndex(VertSel->BodyIndex) &&
                ProfileBeingEdited->Bodies[VertSel->BodyIndex].Shapes.IsValidIndex(VertSel->ShapeIndex))
            {
                FBox2DCollisionShape& Shape = ProfileBeingEdited->Bodies[VertSel->BodyIndex].Shapes[VertSel->ShapeIndex];
                if (Shape.ShapeType == EBox2DCollisionShapeType::Polygon && Shape.Vertices.IsValidIndex(VertSel->VertexIndex))
                {
                    Shape.Vertices.RemoveAt(VertSel->VertexIndex);
                    if (Shape.Vertices.Num() < 3)
                    {
                        ShapesToDelete.Add(TPair<int32, int32>(VertSel->BodyIndex, VertSel->ShapeIndex));
                    }
                }
                else
                {
                    ShapesToDelete.Add(TPair<int32, int32>(VertSel->BodyIndex, VertSel->ShapeIndex));
                }
            }
        }
    }

    // Delete shapes (reverse order within each body)
    for (const TPair<int32, int32>& Pair : ShapesToDelete)
    {
        if (BodiesToDelete.Contains(Pair.Key)) continue; // Body will be deleted entirely
        if (ProfileBeingEdited->Bodies.IsValidIndex(Pair.Key))
        {
            ProfileBeingEdited->Bodies[Pair.Key].Shapes.RemoveAt(Pair.Value);
        }
    }

    // Delete bodies (reverse order)
    TArray<int32> SortedBodies = BodiesToDelete.Array();
    SortedBodies.Sort([](int32 A, int32 B) { return A > B; });
    for (int32 BodyIdx : SortedBodies)
    {
        ProfileBeingEdited->RemoveBody(BodyIdx);
    }

    ClearSelection();
}

void FBox2DCollisionGeometryEditMode::AddBoxShape()
{
    if (!ProfileBeingEdited) return;

    // Ensure there's at least one body
    if (ProfileBeingEdited->Bodies.Num() == 0)
    {
        ProfileBeingEdited->AddDefaultBody();
    }

    const FScopedTransaction Transaction(LOCTEXT("AddBoxShape", "Add Box Shape"));
    ProfileBeingEdited->Modify();

    int32 TargetBody = FMath::Clamp(ActiveBodyIndex, 0, ProfileBeingEdited->Bodies.Num() - 1);
    FBox2DCollisionShape NewShape;
    NewShape.ShapeType = EBox2DCollisionShapeType::Box;
    NewShape.HalfExtents = FVector2D(0.5f, 0.5f);
    ProfileBeingEdited->Bodies[TargetBody].Shapes.Add(NewShape);

    ClearSelection();
    AddToSelection(MakeShared<FBox2DSelectedShape>(TargetBody, ProfileBeingEdited->Bodies[TargetBody].Shapes.Num() - 1, ProfileBeingEdited));
}

void FBox2DCollisionGeometryEditMode::AddCircleShape()
{
    if (!ProfileBeingEdited) return;

    if (ProfileBeingEdited->Bodies.Num() == 0)
    {
        ProfileBeingEdited->AddDefaultBody();
    }

    const FScopedTransaction Transaction(LOCTEXT("AddCircleShape", "Add Circle Shape"));
    ProfileBeingEdited->Modify();

    int32 TargetBody = FMath::Clamp(ActiveBodyIndex, 0, ProfileBeingEdited->Bodies.Num() - 1);
    FBox2DCollisionShape NewShape;
    NewShape.ShapeType = EBox2DCollisionShapeType::Circle;
    NewShape.HalfExtents = FVector2D(0.5f, 0.0f);
    ProfileBeingEdited->Bodies[TargetBody].Shapes.Add(NewShape);

    ClearSelection();
    AddToSelection(MakeShared<FBox2DSelectedShape>(TargetBody, ProfileBeingEdited->Bodies[TargetBody].Shapes.Num() - 1, ProfileBeingEdited));
}

void FBox2DCollisionGeometryEditMode::ToggleAddPolygonMode()
{
    if (bIsAddingPolygon)
    {
        FinishPolygon();
    }
    else
    {
        bIsAddingPolygon = true;
        PolygonPoints.Empty();
    }
}

void FBox2DCollisionGeometryEditMode::AbandonAddPolygon()
{
    bIsAddingPolygon = false;
    PolygonPoints.Empty();
}

void FBox2DCollisionGeometryEditMode::FinishPolygon()
{
    if (!ProfileBeingEdited || PolygonPoints.Num() < 3)
    {
        AbandonAddPolygon();
        return;
    }

    const FScopedTransaction Transaction(LOCTEXT("AddPolygonShape", "Add Polygon Shape"));
    ProfileBeingEdited->Modify();

    if (ProfileBeingEdited->Bodies.Num() == 0)
    {
        ProfileBeingEdited->AddDefaultBody();
    }

    int32 TargetBody = FMath::Clamp(ActiveBodyIndex, 0, ProfileBeingEdited->Bodies.Num() - 1);

    // Compute centroid as shape center
    FVector2D Centroid = FVector2D::ZeroVector;
    for (const FVector2D& Pt : PolygonPoints)
    {
        Centroid += Pt;
    }
    Centroid /= PolygonPoints.Num();

    FBox2DCollisionShape NewShape;
    NewShape.ShapeType = EBox2DCollisionShapeType::Polygon;
    NewShape.Center = Centroid;
    for (const FVector2D& Pt : PolygonPoints)
    {
        NewShape.Vertices.Add(Pt - Centroid);
    }
    ProfileBeingEdited->Bodies[TargetBody].Shapes.Add(NewShape);

    ClearSelection();
    AddToSelection(MakeShared<FBox2DSelectedShape>(TargetBody, ProfileBeingEdited->Bodies[TargetBody].Shapes.Num() - 1, ProfileBeingEdited));

    bIsAddingPolygon = false;
    PolygonPoints.Empty();
}

void FBox2DCollisionGeometryEditMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
    FEdMode::Tick(ViewportClient, DeltaTime);
}

bool FBox2DCollisionGeometryEditMode::ShouldDrawWidget() const
{
    return HasSelection();
}

FVector FBox2DCollisionGeometryEditMode::GetWidgetLocation() const
{
    if (SelectedItems.Num() == 0) return FVector::ZeroVector;

    FVector Sum(ForceInitToZero);
    for (const TSharedPtr<FBox2DSelectedItem>& Item : SelectedItems)
    {
        Sum += Item->GetWorldPos();
    }
    return Sum / SelectedItems.Num();
}

bool FBox2DCollisionGeometryEditMode::UsesTransformWidget(UE::Widget::EWidgetMode CheckMode) const
{
    return CheckMode == UE::Widget::WM_Translate || CheckMode == UE::Widget::WM_None;
}

bool FBox2DCollisionGeometryEditMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
    bool bHandled = false;

    const bool bManipulating = GetModeManager()->IsTracking();
    EAxisList::Type CurrentAxis = GetCurrentWidgetAxis();

    if (bManipulating && CurrentAxis != EAxisList::None && SelectedItems.Num() > 0)
    {
        bHandled = true;

        // Convert 3D drag on XZ plane to 2D delta
        FVector2D Drag2D(InDrag.X, InDrag.Z);

        const FScopedTransaction Transaction(LOCTEXT("MoveSelection", "Move Selection"));
        if (ProfileBeingEdited)
        {
            ProfileBeingEdited->Modify();
        }

        for (TSharedPtr<FBox2DSelectedItem>& Item : SelectedItems)
        {
            Item->ApplyDelta(Drag2D, InRot, InScale);
        }
    }

    return bHandled ? true : FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
}

bool FBox2DCollisionGeometryEditMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
    if (!ProfileBeingEdited) return false;

    FViewport* Viewport = InViewportClient->Viewport;
    const bool bIsCtrlKeyDown = Viewport->KeyState(EKeys::LeftControl) || Viewport->KeyState(EKeys::RightControl);
    const bool bIsShiftKeyDown = Viewport->KeyState(EKeys::LeftShift) || Viewport->KeyState(EKeys::RightShift);

    // Handle polygon creation clicks
    if (bIsAddingPolygon)
    {
        if (Click.GetKey() == EKeys::LeftMouseButton && Click.GetEvent() == IE_Pressed)
        {
            FVector2D ClickPos2D = ViewportClickTo2D(Click, InViewportClient);
            PolygonPoints.Add(ClickPos2D);
            return true;
        }
        return false;
    }

    // Convert click to 2D space
    FVector2D ClickPos2D = ViewportClickTo2D(Click, InViewportClient);

    if (Click.GetEvent() != IE_Pressed) return false;

    bool bHandled = false;

    // Try to find what was clicked
    float Scale = ProfileBeingEdited->PixelsPerMeter;
    float BestDist = 15.0f; // Click tolerance in pixels (approximate)
    TSharedPtr<FBox2DSelectedItem> BestHit;

    // Check body origins first
    for (int32 BodyIdx = 0; BodyIdx < ProfileBeingEdited->Bodies.Num(); BodyIdx++)
    {
        const FBox2DCollisionBody& Body = ProfileBeingEdited->Bodies[BodyIdx];
        FVector2D BodyWorldPos = Body.Position * Scale;

        // Simple distance check
        FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
            Viewport, InViewportClient->GetScene(), InViewportClient->EngineShowFlags));
        FSceneView* View = InViewportClient->CalcSceneView(&ViewFamily);
        FVector2D ScreenPos;
        View->WorldToPixel(ToXZPlane(BodyWorldPos), ScreenPos);
        FVector2D ClickScreen(Click.GetCursorPos().X, Click.GetCursorPos().Y);
        float Dist = (ScreenPos - ClickScreen).Size();

        if (Dist < BestDist)
        {
            BestDist = Dist;
            BestHit = MakeShared<FBox2DSelectedBody>(BodyIdx, ProfileBeingEdited);
        }

        // Check shapes
        for (int32 ShapeIdx = 0; ShapeIdx < Body.Shapes.Num(); ShapeIdx++)
        {
            const FBox2DCollisionShape& Shape = Body.Shapes[ShapeIdx];
            FVector2D ShapeWorldPos = (Body.Position + Shape.Center) * Scale;

            View->WorldToPixel(ToXZPlane(ShapeWorldPos), ScreenPos);
            Dist = (ScreenPos - ClickScreen).Size();

            if (Dist < BestDist)
            {
                BestDist = Dist;
                BestHit = MakeShared<FBox2DSelectedShape>(BodyIdx, ShapeIdx, ProfileBeingEdited);
            }

            // Check polygon vertices
            if (Shape.ShapeType == EBox2DCollisionShapeType::Polygon)
            {
                for (int32 VertIdx = 0; VertIdx < Shape.Vertices.Num(); VertIdx++)
                {
                    FVector2D VertWorldPos = (Body.Position + Shape.Center + Shape.Vertices[VertIdx]) * Scale;
                    View->WorldToPixel(ToXZPlane(VertWorldPos), ScreenPos);
                    Dist = (ScreenPos - ClickScreen).Size();
                    if (Dist < BestDist)
                    {
                        BestDist = Dist;
                        BestHit = MakeShared<FBox2DSelectedVertex>(BodyIdx, ShapeIdx, VertIdx, ProfileBeingEdited);
                    }
                }
            }
        }
    }

    if (BestHit.IsValid())
    {
        if (!bIsCtrlKeyDown && !bIsShiftKeyDown)
        {
            ClearSelection();
        }
        AddToSelection(BestHit);
        bHandled = true;
    }
    else
    {
        // Clicked on empty space - deselect
        if (!bIsShiftKeyDown)
        {
            ClearSelection();
        }
    }

    // Double-click selects whole shape from vertex
    if (bHandled && Click.GetEvent() == IE_DoubleClick)
    {
        if (SelectedItems.Num() == 1)
        {
            if (const FBox2DSelectedVertex* VertSel = SelectedItems[0]->CastTo<FBox2DSelectedVertex>(FBox2DCollisionSelectionTypes::Vertex))
            {
                ClearSelection();
                AddToSelection(MakeShared<FBox2DSelectedShape>(VertSel->BodyIndex, VertSel->ShapeIndex, ProfileBeingEdited));
            }
        }
    }

    return bHandled ? true : FEdMode::HandleClick(InViewportClient, HitProxy, Click);
}

bool FBox2DCollisionGeometryEditMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
    bool bHandled = false;

    if (bIsAddingPolygon)
    {
        if (Key == EKeys::Enter && Event == IE_Pressed)
        {
            FinishPolygon();
            bHandled = true;
        }
        else if (Key == EKeys::Escape && Event == IE_Pressed)
        {
            AbandonAddPolygon();
            bHandled = true;
        }
        else if (Key == EKeys::BackSpace && Event == IE_Pressed && PolygonPoints.Num() > 0)
        {
            PolygonPoints.Pop();
            bHandled = true;
        }
    }

    // Delete key
    if (Key == EKeys::Delete && Event == IE_Pressed && HasSelection())
    {
        DeleteSelectedItems();
        bHandled = true;
    }

    return bHandled ? true : FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

void FBox2DCollisionGeometryEditMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
    FEdMode::Render(View, Viewport, PDI);

    DrawSelectionHighlight(PDI);

    if (bIsAddingPolygon)
    {
        DrawPolygonPreview(PDI);
    }
}

void FBox2DCollisionGeometryEditMode::DrawSelectionHighlight(FPrimitiveDrawInterface* PDI)
{
    for (const TSharedPtr<FBox2DSelectedItem>& Item : SelectedItems)
    {
        if (const FBox2DSelectedShape* ShapeSel = Item->CastTo<FBox2DSelectedShape>(FBox2DCollisionSelectionTypes::Shape))
        {
            if (!ProfileBeingEdited) continue;
            if (!ProfileBeingEdited->Bodies.IsValidIndex(ShapeSel->BodyIndex)) continue;
            const FBox2DCollisionBody& Body = ProfileBeingEdited->Bodies[ShapeSel->BodyIndex];
            if (!Body.Shapes.IsValidIndex(ShapeSel->ShapeIndex)) continue;
            const FBox2DCollisionShape& Shape = Body.Shapes[ShapeSel->ShapeIndex];

            float Scale = ProfileBeingEdited->PixelsPerMeter;
            FVector2D Center = (Body.Position + Shape.Center) * Scale;
            float RotRad = FMath::DegreesToRadians(Shape.Rotation + Body.Rotation);

            if (Shape.ShapeType == EBox2DCollisionShapeType::Box)
            {
                FVector2D HalfExt = Shape.HalfExtents * Scale;
                FVector2D LocalCorners[4] = {
                    FVector2D(-HalfExt.X, -HalfExt.Y), FVector2D(HalfExt.X, -HalfExt.Y),
                    FVector2D(HalfExt.X, HalfExt.Y), FVector2D(-HalfExt.X, HalfExt.Y)
                };
                FVector Corners3D[4];
                for (int32 i = 0; i < 4; i++)
                {
                    float C = FMath::Cos(RotRad), S = FMath::Sin(RotRad);
                    FVector2D Rotated(LocalCorners[i].X*C - LocalCorners[i].Y*S, LocalCorners[i].X*S + LocalCorners[i].Y*C);
                    Corners3D[i] = ToXZPlane(Center + Rotated);
                }
                for (int32 i = 0; i < 4; i++)
                {
                    PDI->DrawLine(Corners3D[i], Corners3D[(i+1)%4], Box2DEditModeColors::SelectionColor, SDPG_Foreground);
                }
            }
            else if (Shape.ShapeType == EBox2DCollisionShapeType::Circle)
            {
                float Radius = Shape.HalfExtents.X * Scale;
                const int32 Segs = 32;
                for (int32 i = 0; i < Segs; i++)
                {
                    float A1 = 2*PI*i/Segs, A2 = 2*PI*(i+1)/Segs;
                    PDI->DrawLine(
                        ToXZPlane(Center + FVector2D(Radius*FMath::Cos(A1), Radius*FMath::Sin(A1))),
                        ToXZPlane(Center + FVector2D(Radius*FMath::Cos(A2), Radius*FMath::Sin(A2))),
                        Box2DEditModeColors::SelectionColor, SDPG_Foreground);
                }
            }
            else if (Shape.ShapeType == EBox2DCollisionShapeType::Polygon && Shape.Vertices.Num() >= 3)
            {
                for (int32 i = 0; i < Shape.Vertices.Num(); i++)
                {
                    float C = FMath::Cos(RotRad), S = FMath::Sin(RotRad);
                    FVector2D V1Local = Shape.Vertices[i] * Scale;
                    FVector2D V2Local = Shape.Vertices[(i+1) % Shape.Vertices.Num()] * Scale;
                    FVector2D V1(C*V1Local.X - S*V1Local.Y, S*V1Local.X + C*V1Local.Y);
                    FVector2D V2(C*V2Local.X - S*V2Local.Y, S*V2Local.X + C*V2Local.Y);
                    PDI->DrawLine(
                        ToXZPlane(Center + V1), ToXZPlane(Center + V2),
                        Box2DEditModeColors::SelectionColor, SDPG_Foreground);
                }
            }
        }
        else if (const FBox2DSelectedVertex* VertSel = Item->CastTo<FBox2DSelectedVertex>(FBox2DCollisionSelectionTypes::Vertex))
        {
            PDI->DrawPoint(VertSel->GetWorldPos(), Box2DEditModeColors::SelectionColor,
                Box2DEditModeColors::VertexSize, SDPG_Foreground);
        }
        else if (const FBox2DSelectedBody* BodySel = Item->CastTo<FBox2DSelectedBody>(FBox2DCollisionSelectionTypes::Body))
        {
            PDI->DrawPoint(BodySel->GetWorldPos(), Box2DEditModeColors::SelectionColor,
                Box2DEditModeColors::VertexSize, SDPG_Foreground);
        }
    }
}

void FBox2DCollisionGeometryEditMode::DrawPolygonPreview(FPrimitiveDrawInterface* PDI)
{
    if (!ProfileBeingEdited || PolygonPoints.Num() == 0) return;
    float Scale = ProfileBeingEdited->PixelsPerMeter;

    for (int32 i = 0; i < PolygonPoints.Num(); i++)
    {
        FVector Pt = ToXZPlane(PolygonPoints[i] * Scale);
        PDI->DrawPoint(Pt, Box2DEditModeColors::PolygonPreviewColor, Box2DEditModeColors::VertexSize, SDPG_Foreground);

        if (i > 0)
        {
            FVector PrevPt = ToXZPlane(PolygonPoints[i-1] * Scale);
            PDI->DrawLine(PrevPt, Pt, Box2DEditModeColors::PolygonPreviewColor, SDPG_Foreground);
        }
    }
}

void FBox2DCollisionGeometryEditMode::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
    FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);

    if (bIsAddingPolygon)
    {
        FCanvasTextItem TextItem(FVector2D(6, 24),
            LOCTEXT("PolygonMode", "Polygon Mode: Click to add points, Enter to finish, Escape to cancel"),
            GEngine->GetSmallFont(), Box2DEditModeColors::PolygonPreviewColor);
        TextItem.EnableShadow(FLinearColor::Black);
        TextItem.Draw(Canvas);
    }
}

#undef LOCTEXT_NAMESPACE
