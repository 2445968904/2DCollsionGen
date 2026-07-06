#include "Box2DCollisionEditorPCH.h"
#include "Box2DCollisionProfileDetailsCustomization.h"
#include "Box2DCollisionProfile.h"
#include "Box2DProjectionUtils.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "ScopedTransaction.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "Box2DCollisionProfileDetails"

TSharedRef<IDetailCustomization> FBox2DCollisionProfileDetailsCustomization::MakeInstance()
{
    return MakeShareable(new FBox2DCollisionProfileDetailsCustomization);
}

void FBox2DCollisionProfileDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // Cache the profile being edited
    TArray<TWeakObjectPtr<UObject>> SelectedObjects;
    DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
    for (TWeakObjectPtr<UObject>& Obj : SelectedObjects)
    {
        if (Obj.IsValid() && Obj->IsA<UBox2DCollisionProfile>())
        {
            CachedProfile = Cast<UBox2DCollisionProfile>(Obj.Get());
            break;
        }
    }

    IDetailCategoryBuilder& SourceCategory = DetailBuilder.EditCategory("AutoGenerate", LOCTEXT("AutoGenerateCat", "Auto-Generate Collision"));

    SourceCategory.AddCustomRow(LOCTEXT("AutoGenerateRow", "Auto-Generate"))
    .WholeRowContent()
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(2)
        [
            SNew(SButton)
            .Text(LOCTEXT("GenSingleBox", "Generate Single Box"))
            .ToolTipText(LOCTEXT("GenSingleBoxTip", "Generate a single box collision shape from the source mesh bounding box."))
            .OnClicked(this, &FBox2DCollisionProfileDetailsCustomization::OnAutoGenerateSingleBox)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(2)
        [
            SNew(SButton)
            .Text(LOCTEXT("GenSingleCircle", "Generate Single Circle"))
            .ToolTipText(LOCTEXT("GenSingleCircleTip", "Generate a single circle collision shape from the source mesh bounding sphere."))
            .OnClicked(this, &FBox2DCollisionProfileDetailsCustomization::OnAutoGenerateSingleCircle)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(2)
        [
            SNew(SButton)
            .Text(LOCTEXT("GenConvexPoly", "Generate Convex Polygon"))
            .ToolTipText(LOCTEXT("GenConvexPolyTip", "Generate a convex polygon collision shape from the source mesh (max 8 vertices)."))
            .OnClicked(this, &FBox2DCollisionProfileDetailsCustomization::OnAutoGenerateConvexPolygon)
        ]
    ];
}

UBox2DCollisionProfile* FBox2DCollisionProfileDetailsCustomization::GetProfile() const
{
    return CachedProfile.Get();
}

FReply FBox2DCollisionProfileDetailsCustomization::OnAutoGenerateSingleBox()
{
    UBox2DCollisionProfile* Profile = GetProfile();
    if (!Profile) return FReply::Handled();

    UStaticMesh* SourceMesh = Profile->SourceMesh.LoadSynchronous();
    if (!SourceMesh)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSourceMesh", "No Source Mesh assigned. Please assign a StaticMesh first."));
        return FReply::Handled();
    }

    FScopedTransaction Transaction(LOCTEXT("AutoGenBox", "Auto-Generate Box Collision"));
    Profile->Modify();

    if (!Box2DProjectionUtils::AutoGenerateCollision(Profile, SourceMesh, Box2DProjectionUtils::EAutoGenMethod::SingleBox))
    {
        Transaction.Cancel();
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("AutoGenFailed", "Failed to generate collision. The mesh may have no vertex data."));
    }

    return FReply::Handled();
}

FReply FBox2DCollisionProfileDetailsCustomization::OnAutoGenerateSingleCircle()
{
    UBox2DCollisionProfile* Profile = GetProfile();
    if (!Profile) return FReply::Handled();

    UStaticMesh* SourceMesh = Profile->SourceMesh.LoadSynchronous();
    if (!SourceMesh)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSourceMesh", "No Source Mesh assigned. Please assign a StaticMesh first."));
        return FReply::Handled();
    }

    FScopedTransaction Transaction(LOCTEXT("AutoGenCircle", "Auto-Generate Circle Collision"));
    Profile->Modify();

    if (!Box2DProjectionUtils::AutoGenerateCollision(Profile, SourceMesh, Box2DProjectionUtils::EAutoGenMethod::SingleCircle))
    {
        Transaction.Cancel();
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("AutoGenFailed", "Failed to generate collision. The mesh may have no vertex data."));
    }

    return FReply::Handled();
}

FReply FBox2DCollisionProfileDetailsCustomization::OnAutoGenerateConvexPolygon()
{
    UBox2DCollisionProfile* Profile = GetProfile();
    if (!Profile) return FReply::Handled();

    UStaticMesh* SourceMesh = Profile->SourceMesh.LoadSynchronous();
    if (!SourceMesh)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoSourceMesh", "No Source Mesh assigned. Please assign a StaticMesh first."));
        return FReply::Handled();
    }

    FScopedTransaction Transaction(LOCTEXT("AutoGenConvex", "Auto-Generate Convex Collision"));
    Profile->Modify();

    if (!Box2DProjectionUtils::AutoGenerateCollision(Profile, SourceMesh, Box2DProjectionUtils::EAutoGenMethod::ConvexPolygon))
    {
        Transaction.Cancel();
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("AutoGenFailed", "Failed to generate collision. The mesh may have no vertex data."));
    }

    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
