#include "Box2DCollisionEditorPCH.h"
#include "CollisionEditor/Box2DBodyList.h"
#include "CollisionEditor/Box2DCollisionEditor.h"
#include "Box2DCollisionProfile.h"
#include "Box2DCollisionTypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SSeparator.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "Box2DBodyList"

void SBox2DBodyList::Construct(const FArguments& InArgs, TSharedPtr<FBox2DCollisionEditor> InEditor)
{
    EditorPtr = InEditor;
    SelectedBodyIndex = INDEX_NONE;
    SelectedJointIndex = INDEX_NONE;

    RefreshListData();

    this->ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .FillHeight(0.5f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                    .Text(LOCTEXT("BodiesHeader", "Bodies"))
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
                    .OnClicked(this, &SBox2DBodyList::OnAddBody)
                    .Content()
                    [
                        SNew(STextBlock)
                        .TextStyle(FAppStyle::Get(), "FlatButtonTextStyle")
                        .Text(LOCTEXT("AddBody", "+"))
                    ]
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
                    .OnClicked(this, &SBox2DBodyList::OnDeleteBody)
                    .IsEnabled(this, &SBox2DBodyList::CanDeleteBody)
                    .Content()
                    [
                        SNew(STextBlock)
                        .TextStyle(FAppStyle::Get(), "FlatButtonTextStyle")
                        .Text(LOCTEXT("DeleteBody", "-"))
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SAssignNew(BodyListView, SListView<TSharedPtr<FString>>)
                .ListItemsSource(&BodyItems)
                .OnGenerateRow(this, &SBox2DBodyList::OnGenerateBodyRow)
                .OnSelectionChanged(this, &SBox2DBodyList::OnBodySelectionChanged)
            ]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SSeparator)
        ]
        + SVerticalBox::Slot()
        .FillHeight(0.5f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(4)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .TextStyle(FAppStyle::Get(), "DetailsView.CategoryTextStyle")
                    .Text(LOCTEXT("JointsHeader", "Joints"))
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
                    .OnClicked(this, &SBox2DBodyList::OnAddJoint)
                    .Content()
                    [
                        SNew(STextBlock)
                        .TextStyle(FAppStyle::Get(), "FlatButtonTextStyle")
                        .Text(LOCTEXT("AddJoint", "+"))
                    ]
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "FlatButton")
                    .OnClicked(this, &SBox2DBodyList::OnDeleteJoint)
                    .IsEnabled(this, &SBox2DBodyList::CanDeleteJoint)
                    .Content()
                    [
                        SNew(STextBlock)
                        .TextStyle(FAppStyle::Get(), "FlatButtonTextStyle")
                        .Text(LOCTEXT("DeleteJoint", "-"))
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SAssignNew(JointListView, SListView<TSharedPtr<FString>>)
                .ListItemsSource(&JointItems)
                .OnGenerateRow(this, &SBox2DBodyList::OnGenerateJointRow)
                .OnSelectionChanged(this, &SBox2DBodyList::OnJointSelectionChanged)
            ]
        ]
    ];
}

UBox2DCollisionProfile* SBox2DBodyList::GetProfile() const
{
    if (EditorPtr.IsValid())
    {
        return EditorPtr.Pin()->GetProfileBeingEdited();
    }
    return nullptr;
}

void SBox2DBodyList::RefreshListData()
{
    BodyItems.Reset();
    JointItems.Reset();

    if (UBox2DCollisionProfile* Profile = GetProfile())
    {
        for (int32 i = 0; i < Profile->Bodies.Num(); i++)
        {
            const FBox2DCollisionBody& Body = Profile->Bodies[i];
            FString TypeName;
            switch (Body.BodyType)
            {
            case EBox2DBodyType::Static: TypeName = TEXT("Static"); break;
            case EBox2DBodyType::Kinematic: TypeName = TEXT("Kinematic"); break;
            case EBox2DBodyType::Dynamic: TypeName = TEXT("Dynamic"); break;
            default: TypeName = TEXT("Unknown"); break;
            }
            BodyItems.Add(MakeShared<FString>(FString::Printf(TEXT("%s (%s, %d shapes)"),
                *Body.BodyName.ToString(), *TypeName, Body.Shapes.Num())));
        }

        for (int32 i = 0; i < Profile->Joints.Num(); i++)
        {
            const FBox2DCollisionJoint& Joint = Profile->Joints[i];
            FString TypeName;
            switch (Joint.JointType)
            {
            case EBox2DCollisionJointType::Distance: TypeName = TEXT("Distance"); break;
            case EBox2DCollisionJointType::Revolute: TypeName = TEXT("Revolute"); break;
            case EBox2DCollisionJointType::Prismatic: TypeName = TEXT("Prismatic"); break;
            case EBox2DCollisionJointType::Weld: TypeName = TEXT("Weld"); break;
            case EBox2DCollisionJointType::Wheel: TypeName = TEXT("Wheel"); break;
            case EBox2DCollisionJointType::Motor: TypeName = TEXT("Motor"); break;
            default: TypeName = TEXT("Unknown"); break;
            }
            JointItems.Add(MakeShared<FString>(FString::Printf(TEXT("%s (%s: %s -> %s)"),
                *Joint.JointName.ToString(), *TypeName,
                *Joint.BodyAName.ToString(), *Joint.BodyBName.ToString())));
        }
    }
}

TSharedRef<ITableRow> SBox2DBodyList::OnGenerateBodyRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        [
            SNew(STextBlock)
            .Text(FText::AsCultureInvariant(*Item.Get()))
            .Margin(FMargin(4, 2))
        ];
}

TSharedRef<ITableRow> SBox2DBodyList::OnGenerateJointRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        [
            SNew(STextBlock)
            .Text(FText::AsCultureInvariant(*Item.Get()))
            .Margin(FMargin(4, 2))
        ];
}

void SBox2DBodyList::OnBodySelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
{
    if (Item.IsValid())
    {
        int32 Idx = BodyItems.Find(Item);
        SelectedBodyIndex = (Idx != INDEX_NONE) ? Idx : INDEX_NONE;
    }
    else
    {
        SelectedBodyIndex = INDEX_NONE;
    }
}

void SBox2DBodyList::OnJointSelectionChanged(TSharedPtr<FString> Item, ESelectInfo::Type SelectInfo)
{
    if (Item.IsValid())
    {
        int32 Idx = JointItems.Find(Item);
        SelectedJointIndex = (Idx != INDEX_NONE) ? Idx : INDEX_NONE;
    }
    else
    {
        SelectedJointIndex = INDEX_NONE;
    }
}

FReply SBox2DBodyList::OnAddBody()
{
    UBox2DCollisionProfile* Profile = GetProfile();
    if (Profile)
    {
        const FScopedTransaction Transaction(LOCTEXT("AddBody", "Add Body"));
        Profile->Modify();
        Profile->AddDefaultBody();
        RefreshListData();
        if (BodyListView.IsValid()) BodyListView->RequestListRefresh();
    }
    return FReply::Handled();
}

FReply SBox2DBodyList::OnDeleteBody()
{
    UBox2DCollisionProfile* Profile = GetProfile();
    if (Profile && Profile->Bodies.IsValidIndex(SelectedBodyIndex))
    {
        const FScopedTransaction Transaction(LOCTEXT("DeleteBody", "Delete Body"));
        Profile->Modify();
        Profile->RemoveBody(SelectedBodyIndex);
        SelectedBodyIndex = INDEX_NONE;
        RefreshListData();
        if (BodyListView.IsValid()) BodyListView->RequestListRefresh();
    }
    return FReply::Handled();
}

bool SBox2DBodyList::CanDeleteBody() const
{
    UBox2DCollisionProfile* Profile = GetProfile();
    return Profile && Profile->Bodies.IsValidIndex(SelectedBodyIndex);
}

FReply SBox2DBodyList::OnAddJoint()
{
    UBox2DCollisionProfile* Profile = GetProfile();
    if (Profile)
    {
        const FScopedTransaction Transaction(LOCTEXT("AddJoint", "Add Joint"));
        Profile->Modify();

        FBox2DCollisionJoint NewJoint;
        NewJoint.JointName = *FString::Printf(TEXT("Joint_%d"), Profile->Joints.Num());
        if (Profile->Bodies.Num() >= 2)
        {
            NewJoint.BodyAName = Profile->Bodies[0].BodyName;
            NewJoint.BodyBName = Profile->Bodies[1].BodyName;
        }
        else if (Profile->Bodies.Num() == 1)
        {
            NewJoint.BodyAName = Profile->Bodies[0].BodyName;
        }
        Profile->Joints.Add(NewJoint);
        RefreshListData();
        if (JointListView.IsValid()) JointListView->RequestListRefresh();
    }
    return FReply::Handled();
}

FReply SBox2DBodyList::OnDeleteJoint()
{
    UBox2DCollisionProfile* Profile = GetProfile();
    if (Profile && Profile->Joints.IsValidIndex(SelectedJointIndex))
    {
        const FScopedTransaction Transaction(LOCTEXT("DeleteJoint", "Delete Joint"));
        Profile->Modify();
        Profile->Joints.RemoveAt(SelectedJointIndex);
        SelectedJointIndex = INDEX_NONE;
        RefreshListData();
        if (JointListView.IsValid()) JointListView->RequestListRefresh();
    }
    return FReply::Handled();
}

bool SBox2DBodyList::CanDeleteJoint() const
{
    UBox2DCollisionProfile* Profile = GetProfile();
    return Profile && Profile->Joints.IsValidIndex(SelectedJointIndex);
}

#undef LOCTEXT_NAMESPACE
