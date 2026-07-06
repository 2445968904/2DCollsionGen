#pragma once

#include "EdMode.h"
#include "CollisionEditor/Box2DCollisionJointSelection.h"

class FCanvas;
class FEditorViewportClient;
class FPrimitiveDrawInterface;
class FSceneView;
class FUICommandList;
class FViewport;
class UBox2DCollisionProfile;

class FBox2DCollisionJointEditMode : public FEdMode
{
public:
    static const FEditorModeID EM_Box2DCollisionJoint;

    FBox2DCollisionJointEditMode();

    // FEdMode interface
    virtual void Initialize() override;
    virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
    virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
    virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
    virtual FVector GetWidgetLocation() const override;
    virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
    virtual bool ShouldDrawWidget() const override;
    virtual bool UsesTransformWidget(UE::Widget::EWidgetMode CheckMode) const override;
    // End of FEdMode interface

    void SetProfileBeingEdited(UBox2DCollisionProfile* InProfile);
    UBox2DCollisionProfile* GetProfileBeingEdited() const { return ProfileBeingEdited; }

    void BindCommands(TSharedPtr<FUICommandList> InCommandList);

    // Selection
    void ClearSelection();
    bool HasSelection() const { return SelectedItems.Num() > 0; }

    // Joint creation
    void AddJoint(EBox2DCollisionJointType JointType);
    void DeleteSelectedItems();

    // Joint creation workflow: click BodyA then BodyB
    void StartCreateJoint(EBox2DCollisionJointType JointType);
    bool IsCreatingJoint() const { return bIsCreatingJoint; }
    void AbandonCreateJoint();

private:
    FVector2D ViewportClickTo2D(const FViewportClick& Click, FEditorViewportClient* ViewportClient) const;
    FVector2D WorldTo2D(const FVector& WorldPos) const;
    FVector TwoDToWorld(const FVector2D& Pos2D) const;

    void DrawJointSelection(FPrimitiveDrawInterface* PDI);

    UBox2DCollisionProfile* ProfileBeingEdited;

    // Selected items
    TArray<TSharedPtr<FBox2DSelectedJointItem>> SelectedItems;

    // Joint creation workflow state
    bool bIsCreatingJoint;
    EBox2DCollisionJointType PendingJointType;
    int32 PendingBodyAIndex;
};
