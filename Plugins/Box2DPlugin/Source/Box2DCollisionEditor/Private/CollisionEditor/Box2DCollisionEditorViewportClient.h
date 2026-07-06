#pragma once

#include "EditorViewportClient.h"
#include "PreviewScene.h"
#include "CollisionEditor/Box2DCollisionEditor.h"

class AStaticMeshActor;
class UStaticMesh;

class FBox2DCollisionEditorViewportClient : public FEditorViewportClient
{
public:
    FBox2DCollisionEditorViewportClient(TWeakPtr<class FBox2DCollisionEditor> InEditor, TWeakPtr<class SEditorViewport> InViewportWidget);

    // FViewportClient interface
    virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
    virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
    virtual void Tick(float DeltaSeconds) override;
    // End of FViewportClient interface

    // FEditorViewportClient interface
    virtual FLinearColor GetBackgroundColor() const override;
    virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
    // End of FEditorViewportClient interface

    // Show toggles
    void ToggleShowGrid() { DrawHelper.bDrawGrid = !DrawHelper.bDrawGrid; Invalidate(); }
    bool IsShowGridChecked() const { return DrawHelper.bDrawGrid; }

    void ToggleShowBounds() { bShowBounds = !bShowBounds; Invalidate(); }
    bool IsShowBoundsChecked() const { return bShowBounds; }

    void ToggleShowCollision() { bShowCollision = !bShowCollision; Invalidate(); }
    bool IsShowCollisionChecked() const { return bShowCollision; }

    void ToggleShowJoints() { bShowJoints = !bShowJoints; Invalidate(); }
    bool IsShowJointsChecked() const { return bShowJoints; }

    void ToggleShowSourceMesh() { bShowSourceMesh = !bShowSourceMesh; UpdatePreviewMesh(); Invalidate(); }
    bool IsShowSourceMeshChecked() const { return bShowSourceMesh; }

    // Mode switching
    void EnterViewMode();
    void EnterEditShapesMode();
    void EnterEditJointsMode();

    bool IsInViewMode() const { return CurrentMode == EBox2DCollisionEditorMode::ViewMode; }
    bool IsInEditShapesMode() const { return CurrentMode == EBox2DCollisionEditorMode::EditShapesMode; }
    bool IsInEditJointsMode() const { return CurrentMode == EBox2DCollisionEditorMode::EditJointsMode; }

    EBox2DCollisionEditorMode::Type GetCurrentMode() const { return CurrentMode; }

    // Focus
    void RequestFocusOnSelection(bool bInstant);

    // Get the profile being edited
    UBox2DCollisionProfile* GetProfileBeingEdited() const;

private:
    FBox ComputeFocusBounds() const;

    // Update the preview mesh actor to match the profile's SourceMesh
    void UpdatePreviewMesh();

private:
    void DrawCollisionShapes(FPrimitiveDrawInterface* PDI, UBox2DCollisionProfile* Profile);
    void DrawJoints(FPrimitiveDrawInterface* PDI, UBox2DCollisionProfile* Profile);
    void DrawCanvasInfo(FViewport& InViewport, FSceneView& View, FCanvas& Canvas, UBox2DCollisionProfile* Profile);

    // Convert Box2D meters to UE world units
    float GetWorldScale() const;

    // The preview scene
    FPreviewScene OwnedPreviewScene;

    // Preview mesh actor in the preview scene
    TWeakObjectPtr<AStaticMeshActor> PreviewMeshActor;

    // Last loaded source mesh (to detect changes)
    TWeakObjectPtr<UStaticMesh> LastSourceMesh;

    // Editor that owns this viewport
    TWeakPtr<FBox2DCollisionEditor> EditorPtr;

    // Pointer back to the viewport widget
    TWeakPtr<class SEditorViewport> ViewportWidgetPtr;

    // Current editor mode
    EBox2DCollisionEditorMode::Type CurrentMode;

    // Show flags
    bool bShowCollision;
    bool bShowJoints;
    bool bShowBounds;
    bool bShowSourceMesh;

    // Deferred zoom
    bool bDeferZoomToProfile;
    bool bDeferZoomIsInstant;
};
