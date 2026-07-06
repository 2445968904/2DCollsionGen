#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DCollisionEditorViewportClient.h"
#include "CollisionEditor/Box2DCollisionEditor.h"
#include "Box2DCollisionProfile.h"
#include "Box2DCollisionTypes.h"
#include "AssetEditorModeManager.h"
#include "SEditorViewport.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "SceneView.h"
#include "EngineUtils.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionEditor"

namespace Box2DEditorConstants
{
    const FLinearColor ShapeColor(0.0f, 0.7f, 1.0f, 1.0f);
    const FLinearColor ShapeFillColor(0.0f, 0.7f, 1.0f, 0.15f);
    const FLinearColor SelectedShapeColor(1.0f, 0.8f, 0.0f, 1.0f);
    const FLinearColor JointColor(0.8f, 0.2f, 0.8f, 1.0f);
    const FLinearColor AnchorColor(1.0f, 0.4f, 0.0f, 1.0f);
    const FLinearColor BodyOriginColor(0.5f, 0.5f, 0.5f, 0.8f);
    const float VertexSize = 4.0f;
    const float AnchorSize = 5.0f;
    const float OriginSize = 3.0f;
    const float DashLength = 10.0f;
    const float DashGap = 5.0f;
}

// Convert 2D position (X=right, Y=up in 2D) to 3D XZ plane (X=right, Z=up, Y=depth)
static FORCEINLINE FVector ToXZPlane(const FVector2D& Pos2D)
{
    return FVector(Pos2D.X, 0.0f, Pos2D.Y);
}

FBox2DCollisionEditorViewportClient::FBox2DCollisionEditorViewportClient(
    TWeakPtr<FBox2DCollisionEditor> InEditor,
    TWeakPtr<SEditorViewport> InViewportWidget)
    : FEditorViewportClient(nullptr, nullptr, InViewportWidget)
    , EditorPtr(InEditor)
    , ViewportWidgetPtr(InViewportWidget)
    , CurrentMode(EBox2DCollisionEditorMode::ViewMode)
    , bShowCollision(true)
    , bShowJoints(true)
    , bShowBounds(false)
    , bDeferZoomToProfile(true)
    , bDeferZoomIsInstant(true)
{
    PreviewScene = &OwnedPreviewScene;
    ((FAssetEditorModeManager*)ModeTools.Get())->SetPreviewScene(PreviewScene);

    SetRealtime(true);

    DrawHelper.bDrawGrid = true;

    EngineShowFlags.DisableAdvancedFeatures();
    EngineShowFlags.SetCompositeEditorPrimitives(true);

    // Use orthographic XZ view (Left) for 2D editing
    SetViewModes(VMI_Lit, VMI_Lit);
    SetViewportType(LVT_OrthoXZ);

    // Set initial perspective view direction (looking from -Y toward +Y)
    SetInitialViewTransform(LVT_Perspective, FVector(0, -100, 0), FRotator(0, 0, 0), DEFAULT_ORTHOZOOM);
}

UBox2DCollisionProfile* FBox2DCollisionEditorViewportClient::GetProfileBeingEdited() const
{
    if (EditorPtr.IsValid())
    {
        return EditorPtr.Pin()->GetProfileBeingEdited();
    }
    return nullptr;
}

float FBox2DCollisionEditorViewportClient::GetWorldScale() const
{
    if (UBox2DCollisionProfile* Profile = GetProfileBeingEdited())
    {
        return Profile->PixelsPerMeter;
    }
    return 100.0f;
}

void FBox2DCollisionEditorViewportClient::Tick(float DeltaSeconds)
{
    // Deferred zoom to profile bounds
    FIntPoint Size = Viewport->GetSizeXY();
    if (bDeferZoomToProfile && (Size.X > 0) && (Size.Y > 0))
    {
        FBox BoundsToFocus = ComputeFocusBounds();
        if (BoundsToFocus.IsValid)
        {
            if (ViewportType != LVT_Perspective)
            {
                TGuardValue<ELevelViewportType> SaveViewportType(ViewportType, LVT_Perspective);
                FocusViewportOnBox(BoundsToFocus, bDeferZoomIsInstant);
            }
            FocusViewportOnBox(BoundsToFocus, bDeferZoomIsInstant);
        }
        bDeferZoomToProfile = false;
    }

    FEditorViewportClient::Tick(DeltaSeconds);
    OwnedPreviewScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

FLinearColor FBox2DCollisionEditorViewportClient::GetBackgroundColor() const
{
    return FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

FBox FBox2DCollisionEditorViewportClient::ComputeFocusBounds() const
{
    UBox2DCollisionProfile* Profile = GetProfileBeingEdited();
    if (!Profile || Profile->Bodies.Num() == 0)
    {
        return FBox(FVector(-100, -1, -100), FVector(100, 1, 100));
    }

    float Scale = GetWorldScale();
    FBox2D Bounds2D(ForceInitToZero);

    for (const FBox2DCollisionBody& Body : Profile->Bodies)
    {
        FVector2D BodyPos = Body.Position * Scale;

        for (const FBox2DCollisionShape& Shape : Body.Shapes)
        {
            FVector2D ShapeCenter = BodyPos + Shape.Center * Scale;

            if (Shape.ShapeType == EBox2DCollisionShapeType::Box)
            {
                FVector2D HalfExt = Shape.HalfExtents * Scale;
                FBox2D ShapeBox(ShapeCenter - HalfExt, ShapeCenter + HalfExt);
                Bounds2D += ShapeBox;
            }
            else if (Shape.ShapeType == EBox2DCollisionShapeType::Circle)
            {
                float Radius = Shape.HalfExtents.X * Scale;
                FBox2D ShapeBox(ShapeCenter - FVector2D(Radius), ShapeCenter + FVector2D(Radius));
                Bounds2D += ShapeBox;
            }
            else if (Shape.ShapeType == EBox2DCollisionShapeType::Polygon && Shape.Vertices.Num() >= 3)
            {
                for (const FVector2D& Vert : Shape.Vertices)
                {
                    Bounds2D += BodyPos + (Shape.Center + Vert) * Scale;
                }
            }
        }
    }

    if (!Bounds2D.bIsValid)
    {
        return FBox(FVector(-100, -1, -100), FVector(100, 1, 100));
    }

    // Add padding
    FVector2D Padding(50.0f);
    Bounds2D.Min -= Padding;
    Bounds2D.Max += Padding;

    // Convert 2D bounds to 3D on XZ plane (Y=0, thin in Y)
    return FBox(
        FVector(Bounds2D.Min.X, -1.0f, Bounds2D.Min.Y),
        FVector(Bounds2D.Max.X, 1.0f, Bounds2D.Max.Y)
    );
}

void FBox2DCollisionEditorViewportClient::RequestFocusOnSelection(bool bInstant)
{
    bDeferZoomToProfile = true;
    bDeferZoomIsInstant = bInstant;
}

void FBox2DCollisionEditorViewportClient::EnterViewMode()
{
    CurrentMode = EBox2DCollisionEditorMode::ViewMode;
    Invalidate();
}

void FBox2DCollisionEditorViewportClient::EnterEditShapesMode()
{
    CurrentMode = EBox2DCollisionEditorMode::EditShapesMode;
    Invalidate();
}

void FBox2DCollisionEditorViewportClient::EnterEditJointsMode()
{
    CurrentMode = EBox2DCollisionEditorMode::EditJointsMode;
    Invalidate();
}

void FBox2DCollisionEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    FEditorViewportClient::Draw(View, PDI);

    UBox2DCollisionProfile* Profile = GetProfileBeingEdited();
    if (!Profile) return;

    if (bShowCollision)
    {
        DrawCollisionShapes(PDI, Profile);
    }

    if (bShowJoints)
    {
        DrawJoints(PDI, Profile);
    }
}

void FBox2DCollisionEditorViewportClient::DrawCollisionShapes(FPrimitiveDrawInterface* PDI, UBox2DCollisionProfile* Profile)
{
    float Scale = GetWorldScale();

    for (int32 BodyIdx = 0; BodyIdx < Profile->Bodies.Num(); BodyIdx++)
    {
        const FBox2DCollisionBody& Body = Profile->Bodies[BodyIdx];
        FVector2D BodyPos = Body.Position * Scale;

        // Draw body origin on XZ plane
        PDI->DrawPoint(ToXZPlane(BodyPos), Box2DEditorConstants::BodyOriginColor, Box2DEditorConstants::OriginSize, SDPG_Foreground);

        for (int32 ShapeIdx = 0; ShapeIdx < Body.Shapes.Num(); ShapeIdx++)
        {
            const FBox2DCollisionShape& Shape = Body.Shapes[ShapeIdx];
            FVector2D ShapeCenter = BodyPos + Shape.Center * Scale;
            float ShapeRotRad = FMath::DegreesToRadians(Shape.Rotation + Body.Rotation);

            FLinearColor Color = Box2DEditorConstants::ShapeColor;

            if (Shape.ShapeType == EBox2DCollisionShapeType::Box)
            {
                FVector2D HalfExt = Shape.HalfExtents * Scale;

                FVector2D LocalCorners[4] = {
                    FVector2D(-HalfExt.X, -HalfExt.Y),
                    FVector2D(HalfExt.X, -HalfExt.Y),
                    FVector2D(HalfExt.X, HalfExt.Y),
                    FVector2D(-HalfExt.X, HalfExt.Y)
                };

                FVector Corners3D[4];
                for (int32 i = 0; i < 4; i++)
                {
                    float CosR = FMath::Cos(ShapeRotRad);
                    float SinR = FMath::Sin(ShapeRotRad);
                    FVector2D Rotated(
                        LocalCorners[i].X * CosR - LocalCorners[i].Y * SinR,
                        LocalCorners[i].X * SinR + LocalCorners[i].Y * CosR
                    );
                    Corners3D[i] = ToXZPlane(ShapeCenter + Rotated);
                }

                for (int32 i = 0; i < 4; i++)
                {
                    PDI->DrawLine(Corners3D[i], Corners3D[(i + 1) % 4], Color, SDPG_Foreground);
                    PDI->DrawPoint(Corners3D[i], Color, Box2DEditorConstants::VertexSize, SDPG_Foreground);
                }
            }
            else if (Shape.ShapeType == EBox2DCollisionShapeType::Circle)
            {
                float Radius = Shape.HalfExtents.X * Scale;
                FVector Center3D = ToXZPlane(ShapeCenter);

                const int32 Segments = 32;
                for (int32 i = 0; i < Segments; i++)
                {
                    float Angle1 = 2.0f * PI * i / Segments + ShapeRotRad;
                    float Angle2 = 2.0f * PI * (i + 1) / Segments + ShapeRotRad;

                    FVector P1 = ToXZPlane(ShapeCenter + FVector2D(Radius * FMath::Cos(Angle1), Radius * FMath::Sin(Angle1)));
                    FVector P2 = ToXZPlane(ShapeCenter + FVector2D(Radius * FMath::Cos(Angle2), Radius * FMath::Sin(Angle2)));

                    PDI->DrawLine(P1, P2, Color, SDPG_Foreground);
                }

                PDI->DrawPoint(Center3D, Color, Box2DEditorConstants::VertexSize, SDPG_Foreground);

                FVector RadiusEnd = ToXZPlane(ShapeCenter + FVector2D(Radius * FMath::Cos(ShapeRotRad), Radius * FMath::Sin(ShapeRotRad)));
                PDI->DrawLine(Center3D, RadiusEnd, Color, SDPG_Foreground);
            }
            else if (Shape.ShapeType == EBox2DCollisionShapeType::Polygon && Shape.Vertices.Num() >= 3)
            {
                TArray<FVector> Verts3D;
                for (const FVector2D& Vert : Shape.Vertices)
                {
                    float CosR = FMath::Cos(ShapeRotRad);
                    float SinR = FMath::Sin(ShapeRotRad);
                    FVector2D LocalVert = (Vert + Shape.Center) * Scale;
                    FVector2D Rotated(
                        LocalVert.X * CosR - LocalVert.Y * SinR,
                        LocalVert.X * SinR + LocalVert.Y * CosR
                    );
                    Verts3D.Add(ToXZPlane(BodyPos + Rotated));
                }

                for (int32 i = 0; i < Verts3D.Num(); i++)
                {
                    PDI->DrawLine(Verts3D[i], Verts3D[(i + 1) % Verts3D.Num()], Color, SDPG_Foreground);
                    PDI->DrawPoint(Verts3D[i], Color, Box2DEditorConstants::VertexSize, SDPG_Foreground);
                }
            }
        }
    }
}

void FBox2DCollisionEditorViewportClient::DrawJoints(FPrimitiveDrawInterface* PDI, UBox2DCollisionProfile* Profile)
{
    float Scale = GetWorldScale();

    for (const FBox2DCollisionJoint& Joint : Profile->Joints)
    {
        int32 BodyAIdx = Profile->FindBodyIndex(Joint.BodyAName);
        int32 BodyBIdx = Profile->FindBodyIndex(Joint.BodyBName);
        if (BodyAIdx == INDEX_NONE || BodyBIdx == INDEX_NONE) continue;

        const FBox2DCollisionBody& BodyA = Profile->Bodies[BodyAIdx];
        const FBox2DCollisionBody& BodyB = Profile->Bodies[BodyBIdx];

        FVector2D AnchorAWorld = (BodyA.Position + Joint.AnchorA) * Scale;
        FVector2D AnchorBWorld = (BodyB.Position + Joint.AnchorB) * Scale;

        FVector AnchorA3D = ToXZPlane(AnchorAWorld);
        FVector AnchorB3D = ToXZPlane(AnchorBWorld);

        FLinearColor Color = Box2DEditorConstants::JointColor;

        if (Joint.JointType == EBox2DCollisionJointType::Distance)
        {
            FVector Dir = AnchorB3D - AnchorA3D;
            float TotalLen = Dir.Size();
            if (TotalLen > 0)
            {
                Dir /= TotalLen;
                float CurrentDist = 0;
                bool bDrawing = true;
                while (CurrentDist < TotalLen)
                {
                    float SegLen = bDrawing ? Box2DEditorConstants::DashLength : Box2DEditorConstants::DashGap;
                    float EndDist = FMath::Min(CurrentDist + SegLen, TotalLen);
                    if (bDrawing)
                    {
                        PDI->DrawLine(AnchorA3D + Dir * CurrentDist, AnchorA3D + Dir * EndDist, Color, SDPG_Foreground);
                    }
                    CurrentDist = EndDist;
                    bDrawing = !bDrawing;
                }
            }
        }
        else
        {
            PDI->DrawLine(AnchorA3D, AnchorB3D, Color, SDPG_Foreground);
        }

        PDI->DrawPoint(AnchorA3D, Box2DEditorConstants::AnchorColor, Box2DEditorConstants::AnchorSize, SDPG_Foreground);
        PDI->DrawPoint(AnchorB3D, Box2DEditorConstants::AnchorColor, Box2DEditorConstants::AnchorSize, SDPG_Foreground);

        FVector MidPoint = (AnchorA3D + AnchorB3D) * 0.5f;
        switch (Joint.JointType)
        {
        case EBox2DCollisionJointType::Revolute:
        {
            const int32 Segs = 16;
            float IndRadius = 5.0f;
            for (int32 i = 0; i < Segs; i++)
            {
                float A1 = 2.0f * PI * i / Segs;
                float A2 = 2.0f * PI * (i + 1) / Segs;
                FVector P1(MidPoint.X + IndRadius * FMath::Cos(A1), 0, MidPoint.Z + IndRadius * FMath::Sin(A1));
                FVector P2(MidPoint.X + IndRadius * FMath::Cos(A2), 0, MidPoint.Z + IndRadius * FMath::Sin(A2));
                PDI->DrawLine(P1, P2, Color, SDPG_Foreground);
            }
            break;
        }
        case EBox2DCollisionJointType::Prismatic:
        {
            FVector ArrowDir = (AnchorB3D - AnchorA3D).GetSafeNormal();
            FVector ArrowEnd = MidPoint + ArrowDir * 8.0f;
            PDI->DrawLine(MidPoint - ArrowDir * 8.0f, ArrowEnd, Color, SDPG_Foreground);
            break;
        }
        case EBox2DCollisionJointType::Weld:
        {
            float S = 4.0f;
            FVector C1(MidPoint.X - S, 0, MidPoint.Z - S);
            FVector C2(MidPoint.X + S, 0, MidPoint.Z - S);
            FVector C3(MidPoint.X + S, 0, MidPoint.Z + S);
            FVector C4(MidPoint.X - S, 0, MidPoint.Z + S);
            PDI->DrawLine(C1, C2, Color, SDPG_Foreground);
            PDI->DrawLine(C2, C3, Color, SDPG_Foreground);
            PDI->DrawLine(C3, C4, Color, SDPG_Foreground);
            PDI->DrawLine(C4, C1, Color, SDPG_Foreground);
            break;
        }
        default:
            break;
        }
    }
}

void FBox2DCollisionEditorViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
    const bool bIsHitTesting = Canvas.IsHitTesting();
    if (!bIsHitTesting)
    {
        Canvas.SetHitProxy(nullptr);
    }

    UBox2DCollisionProfile* Profile = GetProfileBeingEdited();
    if (Profile)
    {
        DrawCanvasInfo(InViewport, View, Canvas, Profile);
    }

    FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
}

void FBox2DCollisionEditorViewportClient::DrawCanvasInfo(FViewport& InViewport, FSceneView& View, FCanvas& Canvas, UBox2DCollisionProfile* Profile)
{
    int32 YPos = 42;

    {
        FCanvasTextItem TextItem(FVector2D(6, YPos), FText::Format(
            LOCTEXT("ProfileInfo", "Bodies: {0}  Joints: {1}  Scale: {2} px/m"),
            FText::AsNumber(Profile->Bodies.Num()),
            FText::AsNumber(Profile->Joints.Num()),
            FText::AsNumber(FMath::RoundToInt(Profile->PixelsPerMeter))),
            GEngine->GetSmallFont(), FLinearColor::White);
        TextItem.EnableShadow(FLinearColor::Black);
        TextItem.Draw(&Canvas);
        YPos += 18;
    }

    if (CurrentMode != EBox2DCollisionEditorMode::ViewMode)
    {
        FText ModeText;
        switch (CurrentMode)
        {
        case EBox2DCollisionEditorMode::EditShapesMode:
            ModeText = LOCTEXT("EditShapesMode", "Mode: Edit Shapes");
            break;
        case EBox2DCollisionEditorMode::EditJointsMode:
            ModeText = LOCTEXT("EditJointsMode", "Mode: Edit Joints");
            break;
        default:
            break;
        }

        FCanvasTextItem TextItem(FVector2D(6, YPos), ModeText, GEngine->GetSmallFont(), FLinearColor::Yellow);
        TextItem.EnableShadow(FLinearColor::Black);
        TextItem.Draw(&Canvas);
        YPos += 18;
    }

    for (int32 i = 0; i < Profile->Bodies.Num(); i++)
    {
        const FBox2DCollisionBody& Body = Profile->Bodies[i];
        FString BodyInfo = FString::Printf(TEXT("  %s (%s, %d shapes)"),
            *Body.BodyName.ToString(),
            Body.BodyType == EBox2DBodyType::Static ? TEXT("Static") :
            Body.BodyType == EBox2DBodyType::Kinematic ? TEXT("Kinematic") : TEXT("Dynamic"),
            Body.Shapes.Num());

        FCanvasTextItem TextItem(FVector2D(6, YPos), FText::AsCultureInvariant(BodyInfo),
            GEngine->GetSmallFont(), Box2DEditorConstants::ShapeColor);
        TextItem.EnableShadow(FLinearColor::Black);
        TextItem.Draw(&Canvas);
        YPos += 14;
    }
}

void FBox2DCollisionEditorViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
    FEditorViewportClient::ProcessClick(View, HitProxy, Key, Event, HitX, HitY);
}

#undef LOCTEXT_NAMESPACE
