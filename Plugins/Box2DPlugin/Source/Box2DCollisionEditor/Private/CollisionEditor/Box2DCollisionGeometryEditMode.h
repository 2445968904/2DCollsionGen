#pragma once

#include "EdMode.h"
#include "CollisionEditor/Box2DCollisionSelection.h"

class FCanvas;
class FEditorViewportClient;
class FPrimitiveDrawInterface;
class FSceneView;
class FUICommandList;
class FViewport;
class UBox2DCollisionProfile;

class FBox2DCollisionGeometryEditMode : public FEdMode
{
public:
    static const FEditorModeID EM_Box2DCollisionGeometry;

    FBox2DCollisionGeometryEditMode();

    // FEdMode interface
    virtual void Initialize() override;
    virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
    virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
    virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
    virtual FVector GetWidgetLocation() const override;
    virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
    virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
    virtual bool ShouldDrawWidget() const override;
    virtual bool UsesTransformWidget(UE::Widget::EWidgetMode CheckMode) const override;
    // End of FEdMode interface

    void SetProfileBeingEdited(UBox2DCollisionProfile* InProfile);
    UBox2DCollisionProfile* GetProfileBeingEdited() const { return ProfileBeingEdited; }

    void BindCommands(TSharedPtr<FUICommandList> InCommandList);

    // Selection
    void ClearSelection();
    bool HasSelection() const { return SelectedItems.Num() > 0; }
    void AddToSelection(TSharedPtr<FBox2DSelectedItem> Item);
    void DeleteSelectedItems();

    // Shape creation
    void AddBoxShape();
    void AddCircleShape();

    // Polygon mode
    void ToggleAddPolygonMode();
    bool IsAddingPolygon() const { return bIsAddingPolygon; }
    void AbandonAddPolygon();
    void FinishPolygon();

    // Can we add shapes?
    bool CanAddShape() const { return ProfileBeingEdited != nullptr; }

private:
    // Convert viewport click to 2D space (meters)
    FVector2D ViewportClickTo2D(const FViewportClick& Click, FEditorViewportClient* ViewportClient) const;
    FVector2D WorldTo2D(const FVector& WorldPos) const;
    FVector TwoDToWorld(const FVector2D& Pos2D) const;

    void DrawSelectionHighlight(FPrimitiveDrawInterface* PDI);
    void DrawPolygonPreview(FPrimitiveDrawInterface* PDI);

    // Profile being edited
    UBox2DCollisionProfile* ProfileBeingEdited;

    // Selected items
    TArray<TSharedPtr<FBox2DSelectedItem>> SelectedItems;

    // Polygon creation mode
    bool bIsAddingPolygon;
    TArray<FVector2D> PolygonPoints;

    // Active body index for adding shapes (-1 = first body)
    int32 ActiveBodyIndex;
};
